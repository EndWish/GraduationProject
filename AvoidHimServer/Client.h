#pragma once

class Room;
class PlayInfo;

enum class ClientState : unsigned int {
	lobby,				// �κ��ϰ��
	roomWait,
	roomReady,
	roomPlay,
	roomWaitComeback	// ��Ⱑ ������ �ٽ� ���ƿ��� ���� ��ٸ��� ����
};

class Client {
private:
	UINT clientID;
	SOCKET socket;

	string name;
	ClientState state;
	Room* pCurrentRoom;
	PlayInfo* pCurrentPlayInfo;
	
	array<char, BUFSIZE> remainBuffer;
	array<char, BUFSIZE> recvBuffer;
	int recvByte;
	// �ֱ� ���� ��Ŷ�� Ÿ���� ����
	CS_PACKET_TYPE lastPacketType;

	wstring nickname;

public:
	// ������ �� �Ҹ���
	Client(SOCKET _socket);
	~Client();

	// Get + Set �Լ�
	array<char, BUFSIZE>& GetRemainBuffer() { return remainBuffer; }
	void CopyRemainBuffer() { memcpy(remainBuffer.data(), buffer.data(), BUFSIZE); }

	array<char, BUFSIZE>& GetRecvBuffer() { return recvBuffer; }
	
	int& GetRecvByte() { return recvByte; }

	void SetLastPacketType(CS_PACKET_TYPE _type) { lastPacketType = _type; }
	CS_PACKET_TYPE GetLastPacketType() const { return lastPacketType; }

	SOCKET GetSocket() const { return socket; }
	
	const string& GetName() const { return name; }
	void SetName(const string& _name) { name = _name; }

	ClientState GetClientState() const { return state; }
	void SetClientState(ClientState _state) { state = _state; }

	UINT GetClientID() const { return clientID; }
	void SetClientID(UINT _clientID) { clientID = _clientID; }

	Room* GetCurrentRoom() const { return pCurrentRoom; }
	void SetCurrentRoom(Room* _pRoom) { pCurrentRoom = _pRoom; }

	PlayInfo* GetCurrentPlayInfo() const { return pCurrentPlayInfo; }
	void SetCurrentPlayInfo(PlayInfo* _pCurrentPlayInfo) { pCurrentPlayInfo = _pCurrentPlayInfo; }

	const wstring& GetNickname() const { return nickname; }
	void SetNickname(const wstring& _nickname)  { nickname = _nickname; }
	// �Ϲ� �Լ�


};