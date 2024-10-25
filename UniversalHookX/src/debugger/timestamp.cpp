#include "timestamp.h"
#include "signature.h"

std::atomic<bool> time_running(false);
uint32_t cycle_delay_timer = 1;
static Tree tree = Tree();

std::unique_ptr<HANDLE, void(*)(HANDLE)> phThread(
	nullptr,
	[](HANDLE handle) { if (handle != nullptr) CloseHandle(handle); }
);

struct ThreadParams {
	uint32_t address;
};

DWORD WINAPI writeSystemTime(LPVOID lpParam) {
	ThreadParams* params = static_cast<ThreadParams*>(lpParam);

	while (time_running) {
		auto now = std::chrono::system_clock::now();
		auto duration = now.time_since_epoch();

		uint32_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		dwwrite(params->address, milliseconds);
	}
	return 0;
}

Tree& getTree() {
	return tree;
}

ThreadParams params;
void startTimeStamp(std::unique_ptr<Variables>& var_ptr) {
	if (phThread) return;
	params.address = var_ptr->functrace.timestamptime_addr;
	phThread.reset(new HANDLE(CreateThread(
		NULL,                 // 기본 보안 속성
		0,                    // 기본 스택 크기
		writeSystemTime,      // 실행할 함수
		&params,              // 전달할 매개변수
		0,                    // 기본 스레드 생성 플래그
		NULL                  // 스레드 ID
	)));

	if (*phThread == NULL) {
		std::cerr << "Failed to create thread!" << std::endl;
		return;
	}

	time_running = true;
	cycle_delay_timer = 3;
	// timeThread.join();startTimeStamp
}

void endTimeStamp(std::unique_ptr<Variables>& var_ptr) {
	if (!phThread) return;
	tree.printTree(var_ptr, &tree.root);
	time_running = false;
	cycle_delay_timer = 0;
	WaitForSingleObject(*phThread, INFINITE);
	phThread.reset();
}

void analyzeTimeStamp(std::unique_ptr<Variables>& var_ptr) {
	if (time_running) {
		if (cycle_delay_timer > 1) {
			cycle_delay_timer -= 1;
			return;
		}
	}
	else {
		return;
	}

	auto& ft = var_ptr->functrace;
	std::stack<std::pair<uint32_t, uint32_t>> timestampStack;
	std::unordered_map<uint32_t, uint32_t> timestampMatch;

	uint32_t stackdepth = 0;
	uint32_t timestampCount = dwread(ft.timestampCount);
	// std::cout << "count: " << std::dec << timestampCount << "\n";
	TreeNode* currentNode = &tree.root;
	for (size_t i = 0; i < timestampCount>>1; i++) {
		uint32_t func_number = dwread(ft.timestamp_addr + 4 + 8 * i) + var_ptr->functrace.offset;
		uint32_t func_time = dwread(ft.timestamp_addr + 4 + 8 * i + 4);

		if (i == 0) tree.starttime = func_time;
		else if (i == ((timestampCount >> 1) - 1)) tree.endtime = func_time;
		if (!timestampStack.empty() && timestampStack.top().first == func_number && timestampMatch[func_number] % 2 == 1) {
			timestampMatch[func_number] -= 1;
			auto& popped = timestampStack.top();
			uint32_t time_delta = func_time - popped.second;
			for (size_t i = 0; i < stackdepth; i++) {
				// std::cout << " ";
			}
			stackdepth -= 1;
			// std::cout << var_ptr->strtable.str[dwread(ft.timestamp_addr + 4 + 8 * i) + var_ptr->functrace.offset] << " / " << time_delta << "\n";
			timestampStack.pop();

			currentNode = currentNode->parent;
			TreeNode* foundtreenode = currentNode->findChild(func_number);
			if (foundtreenode) {
				std::cout << "found: " << var_ptr->strtable.str[func_number] << "\t" << "/ start: 0x" << std::hex << func_time << " end: 0x" << popped.second << "\n";
				if (time_delta) {
					foundtreenode->func_time += time_delta;
					foundtreenode->func_current_delta = time_delta;
					foundtreenode->call_count += 1;
					foundtreenode->padding = popped.second - tree.starttime;
				}
			}
			else {
				// this must has to be found
			}
		}
		else {
			stackdepth += 1;
			for (size_t i = 0; i < stackdepth; i++) {
				// std::cout << " ";
			}
			// std::cout << var_ptr->strtable.str[dwread(ft.timestamp_addr + 4 + 8 * i) + var_ptr->functrace.offset] << " / " << "input" << "\n";
			timestampMatch[func_number] += 1;
			timestampStack.push(std::pair<uint32_t, uint32_t>(func_number, func_time));

			TreeNode* foundtreenode = currentNode->findChild(func_number);
			if (foundtreenode) {
				currentNode = foundtreenode;
			}
			else {
				TreeNode* added = currentNode->addChild(currentNode, func_number, 0, stackdepth, 0);
				currentNode = added;
			}

		}
	}
}

void ensureEndTimestampThread() {
	if (!phThread) return;
	time_running = false;
	cycle_delay_timer = 0;
	WaitForSingleObject(*phThread, INFINITE);
	phThread.reset();
}