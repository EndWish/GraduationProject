#include "stdafx.h"
#include "Animation.h"
#include "GameObject.h"


AnimationClip::AnimationClip() {
	name = "unknown";
	runTime = 0.f;
	nKeyFrame = 1;
	isLoop = false;
	speedRatio = 1.f;
}
AnimationClip::~AnimationClip() {

}

void AnimationClip::LoadFromFile(ifstream& _file, UINT _nBone) {
	if (!_file) {
		cout << "Animation Load Failed" << "\n";
		return;
	}

	ReadStringBinary(name, _file);	// animationSetName(string)	// 애니메이션 셋의 이름
	_file.read((char*)&runTime, sizeof(float));	// animationSetRuntime(float)	// 애니메이션 셋의 런타임
	_file.read((char*)&isLoop, sizeof(bool));	 // animationSetLoop(bool)		// 애니메이션 반복 여부
	_file.read((char*)&speedRatio, sizeof(float));	 // animationSetLoop(bool)		// 애니메이션 반복 여부
	_file.read((char*)&nKeyFrame, sizeof(UINT));	// animationNKeyFrame(UINT)	// 키프레임의 수

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
	nRepeat = 0;
}
AnimationController::~AnimationController() {

}

void AnimationController::AddTime(float _time) {
	shared_ptr<AnimationClip>& pAniClip = pAniClips[currentAniClipName];

	time += _time * pAniClip->getSpeedRatio();
	float maxTime = pAniClip->GetRunTime();
	if (maxTime <= time) {
		if (pAniClip->IsLoop()) {
			time -= maxTime;
			++nRepeat;
		}
		else {
			time = maxTime;
		}
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

	int keyFrameIndex = min( int(time / pCurrentAniClip->GetRunTime() * pCurrentAniClip->GetNKeyFrame()), int(pCurrentAniClip->GetNKeyFrame() - 1));

	for (UINT boneIndex = 0; boneIndex < nBone; ++boneIndex) {
		_pBones[boneIndex]->SetLocalScale(pCurrentAniClip->GetScale(boneIndex, keyFrameIndex));
		_pBones[boneIndex]->SetLocalRotation(pCurrentAniClip->GetRotation(boneIndex, keyFrameIndex));
		_pBones[boneIndex]->SetLocalPosition(pCurrentAniClip->GetPosition(boneIndex, keyFrameIndex));
		_pBones[boneIndex]->UpdateLocalTransform();
	}

}

void AnimationController::ChangeClip(const string& _name, float _time) {
	if (currentAniClipName == _name)
		return;

	if (pAniClips.contains(_name))
		currentAniClipName = _name;
	else
		cout << "ChangeClip : 잘못된 클립이름 " << _name << "\n";

	nRepeat = 1;
	time = _time;
}

bool AnimationController::IsMaxFrame() const {
	return pAniClips.at(currentAniClipName)->GetRunTime() <= time;\
}
