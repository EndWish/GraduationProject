#include "stdafx.h"
#include "Button.h"
#include "GameFramework.h"
Button::Button(string _imgName, XMFLOAT2 _size, XMFLOAT2 _position, ButtonType _type, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _enable) {
	
	// ���ڸ� �ٶ� ���ϰ� �ϱ� ���� Ŭ���̾�Ʈ â�� left, top�κ��� 0,0, �ݴ����� 2,2 ��ǥ ����
	GameFramework& gameFramework = GameFramework::Instance();
	auto [width, height] = gameFramework.GetClientSize();
	position = _position;
	size = _size;
	type = _type;
	enable = _enable;
	isClick = false;

	XMFLOAT2 ratio{ width / (float)2.f, height / (float)2.f };
	img = make_shared<Image2D>(_imgName, size, position, XMFLOAT2(1, 1), _pDevice, _pCommandList);
	
	// �� ��ư�� ��� �� ��ȣ�� �� �ο��� ���� �ؽ�ó�� �־��ش�.
	if (type == ButtonType::room) {
		float fontSize = C_WIDTH / 40.0f;
		shared_ptr<TextBox> newText = make_shared<TextBox>((WCHAR*)L"�޸յ���ü", D2D1::ColorF(1,1,1,1), XMFLOAT2(_position.x + _size.x/2, _position.y + _size.y/2), XMFLOAT2(_size.x / 2, _size.y / 2), fontSize);
		newText->SetText(L"1/5");
		pTexts.push_back(newText);

		newText = make_shared<TextBox>((WCHAR*)L"�޸յ���ü", D2D1::ColorF(1, 1, 1, 1), _position, XMFLOAT2(_size.x/2, _size.y/2), fontSize);
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
	// �ش� ��ġ���� ������ ���� �ൿ�� �� ��� ��ư�� �÷��̾ ���������� �Ǵ�
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
	// �� ��ư�� ��� ���� ���� ĭ�� ��ư�� �ؽ�Ʈ�� �׸��� �ʴ´�.
	if (enable) {
		for (auto text : pTexts) {
			if (text && state != RoomState::none) text->Render();
		}
	}
}

void RoomButton::UpdateState(UINT _roomID, UINT _participant, RoomState _state) {
	state = _state;
	// ���� �̹����� �������ش�.
	if (state == RoomState::full) {
		pTexts[0]->SetText(L"�� ��");
	}
	else if (state == RoomState::joinable) {
		pTexts[0]->SetText(to_wstring(_participant) + L"/5");
	}
	else if (state == RoomState::started) {
		pTexts[0]->SetText(L"������");
	}

	pTexts[1]->SetText(L"#" + to_wstring(_roomID));
	SetRoomIndex(_roomID);
}

