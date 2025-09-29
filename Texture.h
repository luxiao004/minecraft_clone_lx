#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>



inline void texture(const char* texPath, int id, bool flip, unsigned char* data,GLint type1,GLint type2) {
        unsigned int texture1;
        int width, height, nrChannels;
        //T1
        glGenTextures(1, &texture1);//�����������
        stbi_set_flip_vertically_on_load(flip);
        glActiveTexture(33984 + id);//��������Ԫ
        glBindTexture(GL_TEXTURE_2D, texture1);//���������

        //Ϊ��ǰ�󶨶������û��ƣ����ȷ�ʽ
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, type2);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, type2);

        //���ز���������

        data = stbi_load(texPath, &width, &height, &nrChannels, 0);
        if (data) {
            std::cout << "T! yyyyyyes!" << std::endl;
            glTexImage2D(GL_TEXTURE_2D, 0, type1, width, height, 0, type1, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "T1 nnnnnno!" << std::endl;
        }
        stbi_image_free(data);
}
