#include "stdafx.h"
#include "Button.h"
#include "GameFramework.h"
Button::Button(string _imgName, XMFLOAT2 _size, XMFLOAT2 _position, ButtonType _type, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _enable) {
	
	// 인자를 줄때 편하게 하기 위해 클라이언트 창의 left, top부분을 0,0, 반대편을 2,2 좌표 기준
	GameFramework& gameFramework = GameFramework::Instance();
	auto [width, height] = gameFramework.GetClientSize();
	position = _position;
	size = _size;
	type = _type;
	enable = _enable;
	isClick = false;

	XMFLOAT2 ratio{ width / (float)2.f, height / (float)2.f };
	img = make_shared<Image2D>(_imgName, size, position, XMFLOAT2(1, 1), _pDevice, _pCommandList);
	
	// 방 버튼일 경우 방 번호와 방 인원에 대한 텍스처를 넣어준다.
	if (type == ButtonType::room) {
		float fontSize = C_WIDTH / 40.0f;
		shared_ptr<TextBox> newText = make_shared<TextBox>((WCHAR*)L"휴먼돋움체", D2D1::ColorF(1,1,1,1), XMFLOAT2(_position.x + _size.x/2, _position.y + _size.y/2), XMFLOAT2(_size.x / 2, _size.y / 2), fontSize);
		newText->SetText(L"1/5");
		pTexts.push_back(newText);

		newText = make_shared<TextBox>((WCHAR*)L"휴먼돋움체", D2D1::ColorF(1, 1, 1, 1), _position, XMFLOAT2(_size.x/2, _size.y/2), fontSize);
		newText->SetText(L"#1");
		pTexts.push_back(newText);
	}
}

Button::~Button() {

}

bool Button::CheckClick(XMFLOAT2 _pos)
{
	return _pos.x >= (position.x-1) && _pos.x <= (position.x-1) + size.x && _pos.y >= (position.y-1) && _pos.y <= (position.y-1) + size.y;
}

bool Button::CheckEnable()
{
	return enable;
}

bool Button::Press(bool _isClick, XMFLOAT2 _pos)
{
	bool act = false;
	img->SetPress(_isClick);
	// 해당 위치에서 눌렀다 떼는 행동을 한 경우 버튼을 플레이어가 누른것으로 판단
	if (isClick && !_isClick && CheckClick(_pos)) act = true;
	isClick = _isClick;
	return act;
}


void Button::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	if (enable) {
		if(img) img->Render(_pCommandList);
	}
}

void Button::PostRender() {
	if (enable) {
		for (auto text : pTexts) {
			if (text) text->Render();
		}
	}
}

RoomButton::RoomButton(string _imgName, XMFLOAT2 _size, XMFLOAT2 _position, ButtonType _type, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _enable) : Button(_imgName, _size, _position, _type, _pDevice, _pCommandList, _enable) {
	state = RoomState::none;
	roomIndex = 0;
}

RoomButton::~RoomButton() {

}

void RoomButton::PostRender() {
	// 룸 버튼의 경우 방이 없는 칸의 버튼은 텍스트를 그리지 않는다.
	if (enable) {
		for (auto text : pTexts) {
			if (text && state != RoomState::none) text->Render();
		}
	}
}

void RoomButton::UpdateState(UINT _roomID, UINT _participant, RoomState _state) {
	state = _state;
	// 이후 이미지도 변경해준다.
	if (state == RoomState::full) {
		pTexts[0]->SetText(L"꽉 참");
	}
	else if (state == RoomState::joinable) {
		pTexts[0]->SetText(to_wstring(_participant) + L"/5");
	}
	else if (state == RoomState::started) {
		pTexts[0]->SetText(L"시작함");
	}

	pTexts[1]->SetText(L"#" + to_wstring(_roomID));
	SetRoomIndex(_roomID);
}

