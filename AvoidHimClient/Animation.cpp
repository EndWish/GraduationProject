#include "stdafx.h"
#include "Animation.h"
#include "GameObject.h"


AnimationClip::AnimationClip() {
	name = "unknown";
	runTime = 0.f;
	nKeyFrame = 0;
}
AnimationClip::~AnimationClip() {

}

void AnimationClip::LoadFromFile(ifstream& _file, UINT _nBone) {
	if (!_file) {
		cout << "Animation Load Failed" << "\n";
		return;
	}

	ReadStringBinary(name, _file);	// animationSetName(string)	// �ִϸ��̼� ���� �̸�
	_file.read((char*)&runTime, sizeof(float));	// animationSetRuntime(float)	// �ִϸ��̼� ���� ��Ÿ��
	_file.read((char*)&nKeyFrame, sizeof(UINT));	// animationNKeyFrame(UINT)	// Ű�������� ��

	scales.assign(_nBone, vector<XMFLOAT3>(nKeyFrame));
	rotation.assign(_nBone, vector<XMFLOAT4>(nKeyFrame));
	position.assign(_nBone, vector<XMFLOAT3>(nKeyFrame));

	for (UINT boneIndex = 0; boneIndex < _nBone; ++boneIndex) {
		for (UINT keyFrame = 0; keyFrame < nKeyFrame; ++keyFrame) {
			_file.read((char*)&scales[boneIndex][keyFrame], sizeof(XMFLOAT3));
			_file.read((char*)&rotation[boneIndex][keyFrame], sizeof(XMFLOAT4));
			_file.read((char*)&position[boneIndex][keyFrame], sizeof(XMFLOAT3));
		}
	}

}

///////////////////////////////////////////////////////////////////////////////
///

AnimationController::AnimationController() {
	nBone = 0;
	currentAniClipName = "unknown";
	time = 0.f;
}
AnimationController::~AnimationController() {

}

void AnimationController::AddTime(float _time) {
	time += _time;
	float maxTime = pAniClips[currentAniClipName]->GetRunTime();
	if (maxTime <= time) {
		time -= maxTime;
	}
}

void AnimationController::LoadFromFile(ifstream& _file, UINT _nBone) {
	if (!_file) {
		cout << "AnimationController Load Failed" << "\n";
		return;
	}

	nBone = _nBone;

	UINT nAnimationSet = 0;
	_file.read((char*)&nAnimationSet, sizeof(UINT));
	for (int i = 0; i < (int)nAnimationSet; ++i) {
		shared_ptr<AnimationClip> pAniClip = make_shared<AnimationClip>();
		pAniClip->LoadFromFile(_file, _nBone);
		pAniClips[pAniClip->GetName()] = pAniClip;

		if (i == 0)
			currentAniClipName = pAniClip->GetName();
	}


}

void AnimationController::UpdateBoneLocalTransform(vector<shared_ptr<GameObject>>& _pBones) {
	
	
	shared_ptr<AnimationClip> pCurrentAniClip = pAniClips[currentAniClipName];

	int keyFrameIndex = (int)(time / pCurrentAniClip->GetRunTime() * pCurrentAniClip->GetNKeyFrame());

	for (UINT boneIndex = 0; boneIndex < nBone; ++boneIndex) {
		_pBones[boneIndex]->SetLocalScale(pCurrentAniClip->GetScale(boneIndex, keyFrameIndex));
		_pBones[boneIndex]->SetLocalRotation(pCurrentAniClip->GetRotation(boneIndex, keyFrameIndex));
		_pBones[boneIndex]->SetLocalPosition(pCurrentAniClip->GetPosition(boneIndex, keyFrameIndex));
		_pBones[boneIndex]->UpdateLocalTransform();
	}

}