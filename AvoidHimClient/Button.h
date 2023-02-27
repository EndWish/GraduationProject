#pragma once
#include "Image2D.h"

enum class ButtonType : unsigned char { // 
	start,	// 시작 버튼
	room,	// 방 목록을 보여주는 버튼
	exit,	// 종료 버튼
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
	bool enable;				// 버튼이 활성화 된 상태인지 확인
	bool isClick;				// 현재 이 버튼이 눌린 상태인지 확인
	ButtonType type;			// 버튼의 타입
	shared_ptr<Image2D> img;	// 버튼을 그릴 이미지
	XMFLOAT2 position;			// 버튼의 뷰포트 좌표계 기준 위치
	XMFLOAT2 size;				// 버튼의 크기
	vector<shared_ptr<TextBox>> pTexts;	// 버튼 내의 텍스트들의 벡터

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