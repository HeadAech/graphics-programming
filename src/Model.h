//
// Created by Hubert Klonowski on 12/11/2024.
//

#ifndef MODEL_H
#define MODEL_H
#include "Mesh.h"
#include "Shader.h"
#include "assimp/scene.h"


class Model {

public:

    Model(std::string path);
    Model(Mesh mesh);

    void Draw(Shader *shader, unsigned int skyboxTexure);

    void addMesh(Mesh mesh);
    std::vector<Texture> textureLoaded;
    std::vector<Mesh> meshes;


private:


    std::string directory;

    void loadModel(std::string path);

    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
    unsigned int TextureFromFile(const char *path, const std::string &directory);
};



#endif //MODEL_H
