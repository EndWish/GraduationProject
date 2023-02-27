#pragma once
#include "Image2D.h"

enum class ButtonType : unsigned char { // 
	start,	// ���� ��ư
	room,	// �� ����� �����ִ� ��ư
	exit,	// ���� ��ư
	option,
	makeRoom,

	gameStart,
	ready,
	readyCancel,
	quitRoom,
	title,

	nextRoomPage,
	prevRoomPage,
	refreshRoomList,
};

enum class RoomState : unsigned char {
	none,
	joinable,
	started,
	full,
};

class Button	
{
protected:
	bool enable;				// ��ư�� Ȱ��ȭ �� �������� Ȯ��
	bool isClick;				// ���� �� ��ư�� ���� �������� Ȯ��
	ButtonType type;			// ��ư�� Ÿ��
	shared_ptr<Image2D> img;	// ��ư�� �׸� �̹���
	XMFLOAT2 position;			// ��ư�� ����Ʈ ��ǥ�� ���� ��ġ
	XMFLOAT2 size;				// ��ư�� ũ��
	vector<shared_ptr<TextBox>> pTexts;	// ��ư ���� �ؽ�Ʈ���� ����

public:
	Button(string _imgName, XMFLOAT2 _size, XMFLOAT2 _position, ButtonType _type, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _enable = true);
	~Button();
	bool CheckClick(XMFLOAT2 _pos);
	bool GetIsClick() { return isClick; };
	void SetTexture(const string& _name) { img->SetTexture(_name); };
	void SetEnable(bool _enable) { enable = _enable; if (!_enable) isClick = false; };
	bool CheckEnable();
	bool Press(bool _isClick, XMFLOAT2 _pos);

	ButtonType GetType() { return type; };

	
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	virtual void PostRender();
};

class RoomButton : public Button {
private:
	RoomState state;
	UINT roomIndex;
public:
	RoomButton(string _imgName, XMFLOAT2 _size, XMFLOAT2 _position, ButtonType _type, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _enable = true);
	~RoomButton();
	virtual void PostRender();

	UINT GetRoomIndex() { return roomIndex; };
	void SetRoomIndex(UINT _roomID) { roomIndex = _roomID; };
	
	void UpdateState(UINT _roomID, UINT _participant, RoomState _state);
};