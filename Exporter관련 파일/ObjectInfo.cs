using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum OBJECT_TYPE : byte
{
    player,
    obstacle,
    attack,
};

public class ObjectInfo : MonoBehaviour
{
    public AnimationClip[] animationClips;
    public OBJECT_TYPE objectType;

}