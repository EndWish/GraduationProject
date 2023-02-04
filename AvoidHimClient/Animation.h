#pragma once

class GameObject;

class AnimationClip {
protected:
	string name;
	float runTime;
	UINT nKeyFrame;
	vector<vector<XMFLOAT3>> scales, position;
	vector<vector<XMFLOAT4>> rotation;

public:
	AnimationClip();
	~AnimationClip();

	const string& GetName() const { return name; }
	float GetRunTime() const { return runTime; }
	UINT GetNKeyFrame() const { return nKeyFrame; }
	XMFLOAT3 GetScale(int boneIndex, int keyFrameIndex) const { return scales[boneIndex][keyFrameIndex]; }
	XMFLOAT3 GetPosition(int boneIndex, int keyFrameIndex) const { return position[boneIndex][keyFrameIndex]; }
	XMFLOAT4 GetRotation(int boneIndex, int keyFrameIndex) const { return rotation[boneIndex][keyFrameIndex]; }

	void LoadFromFile(ifstream& _file, UINT _nBone);
	

};

class AnimationController {
protected:
	UINT nBone;
	string currentAniClipName;
	unordered_map<string, shared_ptr<AnimationClip>> pAniClips;
	float time;

public:
	AnimationController();
	~AnimationController();

	void AddTime(float _time);
	void LoadFromFile(ifstream& _file, UINT _nBone);
	void UpdateBoneLocalTransform(vector<shared_ptr<GameObject>>& _pBones);

};


