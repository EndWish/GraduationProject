#pragma once
class Room {
private:
	static const int maxParticipant = 5;	// �ִ� ������ ��
private:
	int id;
	SOCKET host;
	vector<SOCKET> participants;

public:
	// ������ �� �Ҹ���
	Room(int _id);
	~Room();

	// Get + Set �Լ�
	int GetID() const { return id; }
	int GetNumOfParticipants() const { return (int)participants.size(); }
	const vector<SOCKET>& GetParticipants() const { return participants; };

	// �Ϲ��Լ�
	bool EnterUser(SOCKET _socket);	// �ο��� ����á�ٸ� false�� �����Ѵ�.
	void LeaveUser(SOCKET _socket);	// �ش� �Լ��� ȣ���ϸ� ���� �ο��� Ȯ���Ͽ� ���� �������� �Ǵ��Ѵ�.

};

