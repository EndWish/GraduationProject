#pragma once

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

	void AddTime(float _time) { time += _time; }
	void LoadFromFile(ifstream& _file, UINT _nBone);

};



