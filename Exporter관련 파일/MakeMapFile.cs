using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.IO;
using System.Text;
using UnityEditor;
using Unity.VisualScripting;
using static UnityEngine.UI.CanvasScaler;

public class MakeMapFile : MonoBehaviour
{
    void BinaryWriteString(string str, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(str.Length);
        binaryWriter.Write(str.ToCharArray());
    }
    void BinaryWriteVector3(Vector3 vector, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(vector.x);
        binaryWriter.Write(vector.y);
        binaryWriter.Write(vector.z);
    }
    void BinaryWriteVector2(Vector2 vector, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(vector.x);
        binaryWriter.Write(vector.y);
    }
    void BinaryWriteQuat(Quaternion vector, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(vector.x);
        binaryWriter.Write(vector.y);
        binaryWriter.Write(vector.z);
        binaryWriter.Write(vector.w);
    }

    void Start()
    {

        DirectoryInfo directoryInfo = new DirectoryInfo("MapBinaryFile/");
        if (directoryInfo.Exists == false)
        {
            directoryInfo.Create();
        }
        BinaryWriter binaryWriter = new BinaryWriter(File.Open("MapBinaryFile/Map", FileMode.Create));

        // nInstance (UINT)
        binaryWriter.Write(transform.childCount);

        for (int i = 0; i < transform.childCount; i++)
        {
            Transform Component = transform.GetChild(i);

            ObjectInfo info = Component.GetComponent<ObjectInfo>();
            // nameSize(UINT) / fileName (string)
            BinaryWriteString(Component.name, binaryWriter);
            // 이후 프리팹의 이름을 get 하여 넣도록 수정
            //Transform t = PrefabUtility.GetCorrespondingObjectFromSource(Component);
            //Debug.Log(t.name);

            // objectType(char)
            binaryWriter.Write((char)info.objectType);

            BinaryWriteVector3(Component.position, binaryWriter);
            BinaryWriteVector3(Component.lossyScale, binaryWriter);
            BinaryWriteQuat(Component.rotation, binaryWriter);
        }
    }
}