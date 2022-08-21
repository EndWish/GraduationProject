using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.IO;
using System.Text;
using UnityEditor;

public class MakeModelFileScript : MonoBehaviour
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
    void BinaryWriteColor(Color c, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(c.r);
        binaryWriter.Write(c.g);
        binaryWriter.Write(c.b);
        binaryWriter.Write(c.a);
    }

    string CreateMaterialBinaryFile(Material material)
    {
        BinaryWriter binaryWriter = new BinaryWriter(File.Open(material.name + "_material", FileMode.Create));

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

        return material.name + "_material";
    }

    string CreateMeshBinaryFile(Mesh mesh, MeshRenderer meshRenderer)
    {
        BinaryWriter binaryWriter = new BinaryWriter(File.Open(mesh.name + "_mesh", FileMode.Create));

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
            BinaryWriteString(CreateMaterialBinaryFile(materials[i]), binaryWriter);
        }

        return mesh.name + "_mesh";
    }
    string CreateObjectBinaryFile()
    {
        BinaryWriter binaryWriter = new BinaryWriter(File.Open(gameObject.name + "_gameobject", FileMode.Create));

        MeshFilter meshFilter =  gameObject.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = gameObject.GetComponent<MeshRenderer>();

        if (meshFilter && meshRenderer) {   // 메쉬가 없는 경우도 처리해줘야한다.
            CreateMeshBinaryFile(meshFilter.sharedMesh, meshRenderer);
        }

        binaryWriter.Flush();
        binaryWriter.Close();

        return gameObject.name + "_gameobject";
    }

    // Start is called before the first frame update
    void Start()
    {
        CreateObjectBinaryFile();
    }

}
