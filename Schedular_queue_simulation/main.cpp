#include <iostream>
using namespace std;

const int READY_QUEUE_LIMIT = 3;

struct PCB {
	int PID;
	PCB* next;
	PCB* jobnext;
	int process_status; // 1: "Job queue", 2: "Wait", 3: "Ready", 4: "Running"
	PCB(int pid, int ps) {
		PID = pid;
		process_status = ps;
	}
};

struct Queue {
	PCB* head;
	PCB* tail;
	int length;

	Queue() {
		head = nullptr;
		tail = nullptr;
		length = 0;
	}
	void addPCBinJob(PCB* pcb_ptr) {
		if (head == nullptr) {
			head = pcb_ptr;
			tail = pcb_ptr;
			pcb_ptr->jobnext = nullptr;
		}
		else {
			tail->jobnext = pcb_ptr;
			tail = pcb_ptr;
			pcb_ptr->jobnext = nullptr;
		}
		length++;
	}
	void addPCB(PCB* pcb_ptr) {
		if (head == nullptr) {
			head = pcb_ptr;
			tail = pcb_ptr;
			pcb_ptr->next = nullptr;
		}
		else {
			tail->next = pcb_ptr;
			tail = pcb_ptr;
			pcb_ptr->next = nullptr;
		}
		length++;
	}
	void printQueue() {
		PCB* iter = this->head;
		while (iter != nullptr) {
			cout << "PCB" << iter->PID << ' ';
			iter = iter->next;
		}
		cout << '\n';
	}
	void printJobQueue() {
		PCB* iter = this->head;
		while (iter != nullptr) {
			cout << "PCB" << iter->PID << ' ';
			iter = iter->jobnext;
		}
		cout << '\n';
	}
	void toReadyTail(Queue& ready) {
		PCB* temp = this->head;
		this->head = temp->next;
		if (ready.head == nullptr) ready.head = temp;
		if(ready.tail != nullptr) ready.tail->next = temp;
		ready.tail = temp;
		temp->next = nullptr;
		temp->process_status = 3;
		this->length--;
		ready.length++;
	}
	void jobToReady(PCB* pcbptr, Queue& ready) {
		if (ready.head == nullptr) ready.head = pcbptr;
		if (ready.tail != nullptr) ready.tail->next = pcbptr;
		ready.tail = pcbptr;
		pcbptr->next = nullptr;
		pcbptr->process_status = 3;
		ready.length++;
	}
};

struct Scheduler {
	PCB* cpu_running;
	Queue job_queue, ready_queue, hdd_queue;
	bool ioInterrupt = false;
	bool twice = false;

	Scheduler() {
		cpu_running = nullptr;
	}
	void printAll() {
		cout << '\n';
		cout << "Now Running : ";
		if (cpu_running !=nullptr)
			cout << "PCB"<<cpu_running->PID;
		cout<< '\n';
		cout << "Ready queue : ";
		ready_queue.printQueue();
		cout << "Device queue : ";
		hdd_queue.printQueue();
		cout << "Job queue : ";
		job_queue.printJobQueue();
		cout << '\n';
	}
	void shortTermSchedule() {
		cpu_running = ready_queue.head;
		ready_queue.head = ready_queue.head->next;
		ready_queue.length--;
		cpu_running->process_status = 4;
	}
	void longTermSchdule() {
		while (ready_queue.length < READY_QUEUE_LIMIT)
		{
			if (hdd_queue.length > 0 && ioInterrupt == false) {
				hdd_queue.toReadyTail(ready_queue);
			}
			else {
				//job queue 상태인 PCB 찾고 없으면 break 있으면 넣기
				PCB* iter = job_queue.head;
				while (iter != nullptr) {
					if (iter->process_status == 1) break;
					iter = iter->jobnext;
				}
				if (iter != nullptr)
				{
					job_queue.jobToReady(iter, ready_queue);
				}
				else break;
			}
		}
	}
	void TerminatePCB(int pid) {
		if (pid == 1 && ioInterrupt == true) return;
		PCB* iter = job_queue.head;
		PCB* pre_iter = nullptr;
		while (iter->PID != pid)
		{
			if (pre_iter == nullptr) {
				pre_iter = job_queue.head;
			}
			else {
				pre_iter = pre_iter->jobnext;
			}
			iter = iter->jobnext;
		}
		if (pre_iter == nullptr) //iter가 잡큐의 헤드
		{
			job_queue.head = iter->jobnext;
			//delete iter;
		}
		else { //iter가 잡큐의 헤드가 아님
			pre_iter->jobnext = iter->jobnext;
			//delete iter;
		}
		cpu_running = nullptr;
		job_queue.length--;
		cout << "#PID00" << pid << " Terminated.";
		printAll();
	}
	void hddInterrupt(){
		cpu_running->process_status = 2;
		hdd_queue.addPCB(cpu_running);
		cpu_running = nullptr;
		ioInterrupt = true;
		twice = true;
		cout << "**HDD Interrput Occured.**";
		printAll();
	}
	void hddInterruptDone() {
		ioInterrupt = false;
		cout << "**HDD Interrput Terminated.**";
		printAll();
	}
	void Run() {
		PCB p1 = PCB(1, 1);
		job_queue.addPCBinJob(&p1);
		PCB p2 = PCB(2, 1);
		job_queue.addPCBinJob(&p2);
		PCB p3 = PCB(3, 1);
		job_queue.addPCBinJob(&p3);
		PCB p4 = PCB(4, 1);
		job_queue.addPCBinJob(&p4);
		PCB p5 = PCB(5, 1);
		job_queue.addPCBinJob(&p5);
		PCB p6 = PCB(6, 1);
		job_queue.addPCBinJob(&p6);
		PCB p7 = PCB(7, 1);
		job_queue.addPCBinJob(&p7);

		cout << "--- Scheduler Started. --- \n";
		printAll();

		while (job_queue.length > 0) {
			longTermSchdule();

			if (cpu_running == nullptr) {
				shortTermSchedule();
			}
			else {
				int pid_running = cpu_running->PID;
				cout << "#PID00" << pid_running << " Running.";
				printAll();
				if (pid_running == 1 && !twice) { // PCB1 수행 중 HDD interrupt 발생
					hddInterrupt();
				}
				if (pid_running == 2) { //PCB2 Running 된 후 hdd interrupt 처리 완료
					hddInterruptDone();
				}
				if (!ioInterrupt) {
					TerminatePCB(pid_running);
				}
			}
		}
		cout << "\n ---- All Process Done ---- \n";
	}
};

int main() {
	Scheduler scheduler;
	scheduler.Run();
}