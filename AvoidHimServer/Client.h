#pragma once

class Room;
class PlayInfo;

enum class ClientState : unsigned int {
	lobby,				// 로비일경우
	roomWait,
	roomReady,
	roomPlay,
	roomWaitComeback	// 경기가 끝나고 다시 돌아오는 것을 기다리는 상태
};

class Client {
private:
	UINT clientID;
	SOCKET socket;
	bool disconnected;

	string name;
	ClientState state;
	Room* pCurrentRoom;
	PlayInfo* pCurrentPlayInfo;
	
	array<char, bufferSize> remainBuffer;
	// 최근 받은 패킷의 타입을 저장
	CS_PACKET_TYPE lastPacketType;

public:
	// 생성자 및 소멸자
	Client(SOCKET _socket);
	~Client();

	// Get + Set 함수
	array<char, bufferSize>& GetRemainBuffer() { return remainBuffer; }
	void CopyRemainBuffer() { memcpy(remainBuffer.data(), buffer.data(), bufferSize); }

	void SetLastPacketType(CS_PACKET_TYPE _type) { lastPacketType = _type; }
	CS_PACKET_TYPE GetLastPacketType() const { return lastPacketType; }

	SOCKET GetSocket() const { return socket; }

	bool IsDisconnected() const { return disconnected; }
	void SetDisconnected(bool _disconnected) { disconnected = _disconnected; }
	
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

	// 일반 함수


};