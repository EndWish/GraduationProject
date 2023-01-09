#pragma once
class Room {
private:
	static const int maxParticipant = 5;	// 최대 참가자 수
private:
	int id;
	SOCKET host;
	vector<SOCKET> participants;

public:
	// 생성자 및 소멸자
	Room(int _id);
	~Room();

	// Get + Set 함수
	int GetID() const { return id; }
	int GetNumOfParticipants() const { return (int)participants.size(); }
	const vector<SOCKET>& GetParticipants() const { return participants; };

	// 일반함수
	bool EnterUser(SOCKET _socket);	// 인원이 가득찼다면 false를 리턴한다.
	void LeaveUser(SOCKET _socket);	// 해당 함수를 호출하면 방의 인원을 확인하여 방을 삭제할지 판단한다.

};

