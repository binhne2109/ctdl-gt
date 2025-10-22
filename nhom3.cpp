//Quản lý Thẻ Từ vựng (Flashcard App)
//Danh sách Liên kết Đôi
//Tạo ứng dụng học từ vựng. Dùng Danh sách Liên kết Đôi để lưu trữ các thẻ từ vựng. Cho phép người dùng dễ dàng chuyển qua/lại giữa các thẻ và xóa thẻ đã học.

#include <iostream>
#include <string>
#include <fstream>
#include <limits>

using namespace std;

struct Card {
	string front; // từ / câu hỏi
	string back;  // nghĩa / đáp án
	Card* prev;
	Card* next;
	Card(const string& f = "", const string& b = "") : front(f), back(b), prev(nullptr), next(nullptr) {}
};

class FlashcardList {
private:
	Card* head;
	Card* tail;
	Card* current;
	int sz;
public:
	FlashcardList(): head(nullptr), tail(nullptr), current(nullptr), sz(0) {}
	~FlashcardList(){ clear(); }

	bool isEmpty() const { return head == nullptr; }
	int size() const { return sz; }

	void add(const string& front, const string& back){
		Card* node = new Card(front, back);
		if(!head){ head = tail = current = node; }
		else {
			tail->next = node;
			node->prev = tail;
			tail = node;
		}
		sz++;
	}

	void insertAfterCurrent(const string& front, const string& back){
		if(!current){ add(front, back); return; }
		Card* node = new Card(front, back);
		Card* nxt = current->next;
		current->next = node;
		node->prev = current;
		node->next = nxt;
		if(nxt) nxt->prev = node; else tail = node;
		sz++;
	}

	bool removeCurrent(){
		if(!current) return false;
		Card* toDel = current;
		Card* p = toDel->prev;
		Card* n = toDel->next;
		if(p) p->next = n; else head = n;
		if(n) n->prev = p; else tail = p;
		// move current
		if(n) current = n;
		else if(p) current = p;
		else current = nullptr;
		delete toDel;
		sz--;
		return true;
	}

	bool moveNext(){
		if(!current || !current->next) return false;
		current = current->next;
		return true;
	}

	bool movePrev(){
		if(!current || !current->prev) return false;
		current = current->prev;
		return true;
	}

	Card* getCurrent(){ return current; }

	void showAll(){
		Card* it = head;
		int idx = 1;
		while(it){
			cout << idx << ". " << it->front << "  ->  " << it->back;
			if(it == current) cout << "  [CURRENT]";
			cout << "\n";
			it = it->next; idx++;
		}
		if(!head) cout << "(Danh sách trống)\n";
	}

	void clear(){
		Card* it = head;
		while(it){
			Card* nx = it->next;
			delete it;
			it = nx;
		}
		head = tail = current = nullptr;
		sz = 0;
	}

	// Persistence: load/save simple text file, each card on two lines (front then back).
	bool saveToFile(const string& path){
		ofstream ofs(path);
		if(!ofs) return false;
		Card* it = head;
		while(it){
			ofs << it->front << "\n";
			ofs << it->back << "\n";
			it = it->next;
		}
		return true;
	}

	bool loadFromFile(const string& path){
		ifstream ifs(path);
		if(!ifs) return false;
		clear();
		string a,b;
		while(true){
			if(!getline(ifs,a)) break;
			if(!getline(ifs,b)) b = ""; // allow missing back
			add(a,b);
		}
		return true;
	}
};

// Utility: read a full line, trimming trailing CR if present (for Windows files)
static string getline_cr(istream& is){
	string s;
	getline(is,s);
	if(!s.empty() && s.back() == '\r') s.pop_back();
	return s;
}

void printHelp(){
	cout << "\n--- Ứng dụng Quản lý Thẻ Từ vựng (Flashcards) ---\n";
	cout << "Các lệnh (gõ chữ, Enter để chọn):\n";
	cout << "  next     : Chuyển tới thẻ kế tiếp\n";
	cout << "  prev     : Quay lại thẻ trước\n";
	cout << "  flip     : Hiện/ẩn nghĩa của thẻ hiện tại\n";
	cout << "  add      : Thêm thẻ mới (sau thẻ hiện tại)\n";
	cout << "  del      : Xóa thẻ hiện tại\n";
	cout << "  list     : Hiện toàn bộ thẻ\n";
	cout << "  save     : Lưu thẻ vào file (flashcards.txt)\n";
	cout << "  load     : Tải thẻ từ file (flashcards.txt)\n";
	cout << "  help     : Hiện hướng dẫn này\n";
	cout << "  quit     : Thoát chương trình\n";
}

int main(){
	FlashcardList deck;
	const string dataFile = "flashcards.txt";

	// Try to load existing file
	if(deck.loadFromFile(dataFile)){
		cout << "Đã tải thẻ từ `" << dataFile << "` (" << deck.size() << " thẻ).\n";
	} else {
		cout << "Không tìm thấy file `" << dataFile << "`. Bắt đầu với danh sách trống.\n";
	}

	printHelp();

	bool showBack = false;
	string cmd;
	while(true){
		Card* cur = deck.getCurrent();
		cout << "\n";
		if(cur){
			cout << "[Thẻ hiện tại] " << cur->front;
			if(showBack) cout << "  ->  " << cur->back;
			cout << "\n";
		} else {
			cout << "(Không có thẻ nào)\n";
		}

		cout << ">> ";
		if(!getline(cin, cmd)) break;
		// trim
		while(!cmd.empty() && isspace((unsigned char)cmd.back())) cmd.pop_back();
		while(!cmd.empty() && isspace((unsigned char)cmd.front())) cmd.erase(cmd.begin());
		if(cmd.empty()) continue;

		if(cmd == "next"){
			if(!deck.moveNext()) cout << "Đây là thẻ cuối, không thể tiến thêm.\n";
			showBack = false;
		} else if(cmd == "prev"){
			if(!deck.movePrev()) cout << "Đây là thẻ đầu, không thể lùi thêm.\n";
			showBack = false;
		} else if(cmd == "flip"){
			showBack = !showBack;
		} else if(cmd == "add"){
			cout << "Nhập mặt trước (từ/câu hỏi):\n> ";
			string f = getline_cr(cin);
			cout << "Nhập mặt sau (nghĩa/đáp án):\n> ";
			string b = getline_cr(cin);
			deck.insertAfterCurrent(f,b);
			cout << "Đã thêm thẻ.\n";
		} else if(cmd == "del"){
			if(deck.removeCurrent()) cout << "Đã xóa thẻ hiện tại.\n";
			else cout << "Không có thẻ để xóa.\n";
		} else if(cmd == "list"){
			deck.showAll();
		} else if(cmd == "save"){
			if(deck.saveToFile(dataFile)) cout << "Đã lưu vào `" << dataFile << "`.\n";
			else cout << "Lỗi khi lưu file.\n";
		} else if(cmd == "load"){
			if(deck.loadFromFile(dataFile)) cout << "Đã tải `" << dataFile << "`.\n";
			else cout << "Không thể mở file `" << dataFile << "`.\n";
		} else if(cmd == "help"){
			printHelp();
		} else if(cmd == "quit"){
			cout << "Bạn có muốn lưu trước khi thoát? (y/n): ";
			string yn; if(!getline(cin,yn)) yn = "n";
			if(!yn.empty() && (yn[0]=='y' || yn[0]=='Y')){
				if(deck.saveToFile(dataFile)) cout << "Đã lưu.\n";
				else cout << "Lưu thất bại.\n";
			}
			break;
		} else {
			cout << "Lệnh không hiểu. Gõ 'help' để xem các lệnh.\n";
		}
	}

	cout << "Tạm biệt!\n";
	return 0;
}

