using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.IO;
using System.Text;
using UnityEditor;

public class MakeModelFileScript : MonoBehaviour
{
    [SerializeField]
    string fileName;
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
    void BinaryWriteQuat(Quaternion vector, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(vector.x);
        binaryWriter.Write(vector.y);
        binaryWriter.Write(vector.z);
        binaryWriter.Write(vector.w);
    }

    void BinaryWriteColor(Color c, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(c.r);
        binaryWriter.Write(c.g);
        binaryWriter.Write(c.b);
        binaryWriter.Write(c.a);
    }
    void BinaryWriteMatrix(Matrix4x4 matrix, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(matrix.m00);
        binaryWriter.Write(matrix.m10);
        binaryWriter.Write(matrix.m20);
        binaryWriter.Write(matrix.m30);
        binaryWriter.Write(matrix.m01);
        binaryWriter.Write(matrix.m11);
        binaryWriter.Write(matrix.m21);
        binaryWriter.Write(matrix.m31);
        binaryWriter.Write(matrix.m02);
        binaryWriter.Write(matrix.m12);
        binaryWriter.Write(matrix.m22);
        binaryWriter.Write(matrix.m32);
        binaryWriter.Write(matrix.m03);
        binaryWriter.Write(matrix.m13);
        binaryWriter.Write(matrix.m23);
        binaryWriter.Write(matrix.m33);
    }

    string CreateMaterialBinaryFile(Material material, string filePath)
    {
        BinaryWriter binaryWriter = new BinaryWriter(File.Open(filePath + material.name + "_material", FileMode.Create));

        // ambient(XMFLOAT4)
        Color ambient = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        BinaryWriteColor(ambient, binaryWriter);
        // diffuse(XMFLOAT4)
        if (material.HasProperty("_Color"))
        {
            Color diffuse = material.GetColor("_Color");
            BinaryWriteColor(diffuse, binaryWriter);
        }
        else
        {
            Color diffuse = new Color(0.0f, 0.0f, 0.0f, 1.0f);
            BinaryWriteColor(diffuse, binaryWriter);
        }
        // specular(XMFLOAT4)(specular.w = 반짝임계수)
        if (material.HasProperty("_SpecColor"))
        {
            Color specular = material.GetColor("_SpecColor");
            BinaryWriteColor(specular, binaryWriter);
        }
        else
        {
            Color specular = new Color(1.0f, 1.0f, 1.0f, 10.0f);
            BinaryWriteColor(specular, binaryWriter);
        }
        // emissive(XMFLOAT4)
        if (material.HasProperty("_EmissionColor"))
        {
            Color emission = material.GetColor("_EmissionColor");
            BinaryWriteColor(emission, binaryWriter);
        }
        else
        {
            Color emission = new Color(0.0f, 0.0f, 0.0f, 1.0f);
            BinaryWriteColor(emission, binaryWriter);
        }
        binaryWriter.Flush();
        binaryWriter.Close();
        return material.name + "_material";
    }

    string CreateMeshBinaryFile(Mesh mesh, MeshRenderer meshRenderer, string filePath)
    {
        BinaryWriter binaryWriter = new BinaryWriter(File.Open(filePath + mesh.name + "_mesh", FileMode.Create));

        // nVertex(UINT)
        binaryWriter.Write((uint)mesh.vertexCount);
        // nameSize (UINT) / name (string)
        BinaryWriteString(mesh.name + "_mesh", binaryWriter);
        // boundingBox (float * 6)
        BinaryWriteVector3(mesh.bounds.center, binaryWriter);
        BinaryWriteVector3(mesh.bounds.extents, binaryWriter);
        Debug.Log(mesh.bounds.center);
        Debug.Log(mesh.bounds.extents);
        // positions (float * 3 * nVertex)
        foreach (Vector3 position in mesh.vertices)
            BinaryWriteVector3(position, binaryWriter);
        // normals (float * 3 * nVertex)
        foreach (Vector3 normal in mesh.normals)
            BinaryWriteVector3(normal, binaryWriter);

        // nSubMesh (UINT)
        Material[] materials = meshRenderer.materials;
        binaryWriter.Write((uint)mesh.subMeshCount);
        for (int i = 0; i < mesh.subMeshCount; i++)
        {
            // nSubMeshIndex (UINT) / subMeshIndex (UINT * nSubMeshIndex)    ....( * nSubMesh )
            int[] subindicies = mesh.GetTriangles(i);
            binaryWriter.Write(subindicies.Length);
            foreach (int index in subindicies)
                binaryWriter.Write(index);
            // materialNameSize(UINT) / materialName(string)
            BinaryWriteString(CreateMaterialBinaryFile(materials[i], filePath), binaryWriter);
        }
        binaryWriter.Flush();
        binaryWriter.Close();
        return mesh.name + "_mesh";
    }
    void CreateObjectBinaryFile(Transform curObjectTransform, BinaryWriter binaryWriter, string filePath)
    {
        string objectName = curObjectTransform.name + "_gameobject";

        // nameSize (UINT) / name(string)
        BinaryWriteString(objectName, binaryWriter);

        //localPosition(float3)
        BinaryWriteVector3(curObjectTransform.localPosition, binaryWriter);

        //localScale(float3)
        BinaryWriteVector3(curObjectTransform.localScale, binaryWriter);

        //localRotation(float4)
        BinaryWriteQuat(curObjectTransform.localRotation, binaryWriter);


        // meshNameSize(UINT) / meshName(string)	=> 메쉬가 없을 경우 따로 처리하자
        MeshFilter meshFilter = curObjectTransform.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = curObjectTransform.GetComponent<MeshRenderer>();

        if (meshFilter && meshRenderer)    // 메쉬가 있는 경우
        {
            BinaryWriteString(CreateMeshBinaryFile(meshFilter.sharedMesh, meshRenderer, filePath), binaryWriter);
        }
        else  // 메쉬가 없는 경우
        {
            binaryWriter.Write(0);
        }

        // nChildren(UINT)
        binaryWriter.Write(curObjectTransform.childCount);
        for (int i = 0; i < curObjectTransform.childCount; i++)  // 자식들을 똑같은 포멧으로 저장
        {
            CreateObjectBinaryFile(curObjectTransform.GetChild(i), binaryWriter, filePath);
        }
    }

    // Start is called before the first frame update
    void Start()
    {

        DirectoryInfo directoryInfo = new DirectoryInfo("ModelBinaryFile/" + gameobject.name);
        if (directoryInfo.Exists == false)
        {
            directoryInfo.Create();
        }
        BinaryWriter binaryWriter = new BinaryWriter(File.Open("ModelBinaryFile/" + gameobject.name + "/" + gameobject.name, FileMode.Create));
        CreateObjectBinaryFile(transform, binaryWriter, "ModelBinaryFile/" + gameobject.name + "/");
        binaryWriter.Flush();
        binaryWriter.Close();

    }

}
