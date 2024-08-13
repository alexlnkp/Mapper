#version 330

/*
    Shader by denysmaistruk (Denys Maistruk)
    https://github.com/denysmaistruk
*/

in vec2 fragTexCoord;

out vec4 finalColor;

uniform sampler2D texture0;

// Input
uniform int size;
uniform int width;
uniform int height;
uniform vec4 color;

void main() {
    float x = 1.0 / width;
    float y = 1.0 / height;

    if (texture(texture0, fragTexCoord).rgb == vec3(0))
    {
        for (int i = -size; i <= size; ++i)
        {
            for (int j = -size; j <= size; ++j)
            {
                if (i == 0 && j == 0)
                {
                    continue;
                }

                if (texture(texture0, fragTexCoord + vec2(i, j) * vec2(x, y)).rgb != vec3(0))
                {
                    finalColor = color;
                    return;
                }
            }
        }
    }
}