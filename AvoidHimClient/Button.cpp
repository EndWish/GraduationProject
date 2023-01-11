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
	
	
	if (type == ButtonType::room) {
		D2D1_RECT_F rect = D2D1::RectF(_position.x * ratio.x + _size.x * ratio.x / 2, _position.y * ratio.y + _size.y * ratio.y / 2, _position.x * ratio.x + _size.x * ratio.x, _position.y * ratio.y + _size.y * ratio.y);
		shared_ptr<TextBox> newText = make_shared<TextBox>((WCHAR*)L"�޸յ���ü", D2D1::ColorF(1,1,1,1), 40.0f, rect);
		newText->SetText(L"1/5");
		texts.push_back(newText);

		rect = D2D1::RectF(_position.x * ratio.x, _position.y * ratio.y, _position.x * ratio.x + _size.x * ratio.x / 2, _position.y * ratio.y + _size.y * ratio.y / 2);
		newText = make_shared<TextBox>((WCHAR*)L"�޸յ���ü", D2D1::ColorF(1, 1, 1, 1), 40.0f, rect);
		newText->SetText(L"#1");
		texts.push_back(newText);
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
		for (auto text : texts) {
			if (text) text->Render();
		}
	}
}

RoomButton::RoomButton(string _imgName, XMFLOAT2 _size, XMFLOAT2 _position, ButtonType _type, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList, bool _enable) : Button(_imgName, _size, _position, _type, _pDevice, _pCommandList, _enable) {
	state = RoomState::none;
}

RoomButton::~RoomButton() {

}

void RoomButton::PostRender() {
	// �� ��ư�� ��� ���� ���� ĭ�� ��ư�� �ؽ�Ʈ�� �׸��� �ʴ´�.
	if (enable) {
		for (auto text : texts) {
			if (text && state != RoomState::none) text->Render();
		}
	}
}

void RoomButton::UpdateState(UINT _roomID, UINT _participant, RoomState _state) {
	state = _state;
	// ���� �̹����� �������ش�.
	if (state == RoomState::full) {
		texts[0]->SetText(L"�� ��");
	}
	else if (state == RoomState::joinable) {
		texts[0]->SetText(to_wstring(_participant) + L"/5");
	}
	else if (state == RoomState::started) {
		texts[0]->SetText(L"������");
	}

	texts[1]->SetText(L"#" + to_wstring(_roomID));
}

