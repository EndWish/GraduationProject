#pragma once
#include "Room.h"
#include "Player.h"

class Scene {
protected:

public:
	Scene();
	virtual ~Scene();

public:
	virtual void FrameAdvance(double _timeElapsed) = 0;
	virtual void AnimateObjects(double _timeElapsed) = 0;
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// PlayScene
class PlayScene : public Scene {
private:
	
	// ��(��) �������� �迭
	vector<shared_ptr<Room>> pRooms;

	// �÷��̾��� ������. ù �÷��� �� ���� �ÿ� �÷��̾ �����Ǿ� ����
	// ��������(��) ��ȯ �ÿ� �� ������ �÷��̾� �����͸� �Ѱ���
	array<shared_ptr<Player>, 2> pPlayer;

	// ���� �÷��̾ �ִ� ���� ������
	array<shared_ptr<Room> , 2> pNowRoom;


public:
	PlayScene(int _stageNum);
	~PlayScene() final;

	void loadRoomsForFile(string _fileName);

public:
	void FrameAdvance(double _timeElapsed) final;
	void AnimateObjects(double _timeElapsed) final;
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) final;

	// ���� �÷��̾ �����ִ� �� ( �浹�˻縦 ������ �� ) �� ã�� �Լ�
	void CheckCurrentRoom(const BoundingOrientedBox& _playerOOBB, int _playerNum);
};