// ----------------------------------------------------------------
// Math library implementation following the asteroids game architecture
// ----------------------------------------------------------------

#include "MathUtils.h"
#include <limits>
#include <cstring>

// Vector2 static variables
const Vector2 Vector2::Zero(0.0f, 0.0f);
const Vector2 Vector2::UnitX(1.0f, 0.0f);
const Vector2 Vector2::UnitY(0.0f, 1.0f);
const Vector2 Vector2::NegUnitX(-1.0f, 0.0f);
const Vector2 Vector2::NegUnitY(0.0f, -1.0f);

// Vector3 static variables
const Vector3 Vector3::Zero(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::UnitX(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::UnitY(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::UnitZ(0.0f, 0.0f, 1.0f);
const Vector3 Vector3::NegUnitX(-1.0f, 0.0f, 0.0f);
const Vector3 Vector3::NegUnitY(0.0f, -1.0f, 0.0f);
const Vector3 Vector3::NegUnitZ(0.0f, 0.0f, -1.0f);
const Vector3 Vector3::Infinity(Math::Infinity, Math::Infinity, Math::Infinity);
const Vector3 Vector3::NegInfinity(Math::NegInfinity, Math::NegInfinity, Math::NegInfinity);

// Matrix4 static variables
const Matrix4 Matrix4::Identity = Matrix4();

// Transform Vector2 by Matrix4
Vector2 Vector2::Transform(const Vector2& vec, const Matrix4& mat, float w)
{
    Vector2 retVal;
    retVal.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[1][0] + w * mat.mat[3][0];
    retVal.y = vec.x * mat.mat[0][1] + vec.y * mat.mat[1][1] + w * mat.mat[3][1];
    return retVal;
}

// Transform Vector3 by Matrix4
Vector3 Vector3::Transform(const Vector3& vec, const Matrix4& mat, float w)
{
    Vector3 retVal;
    retVal.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[1][0] +
               vec.z * mat.mat[2][0] + w * mat.mat[3][0];
    retVal.y = vec.x * mat.mat[0][1] + vec.y * mat.mat[1][1] +
               vec.z * mat.mat[2][1] + w * mat.mat[3][1];
    retVal.z = vec.x * mat.mat[0][2] + vec.y * mat.mat[1][2] +
               vec.z * mat.mat[2][2] + w * mat.mat[3][2];
    return retVal;
}

// This transforms the vector and then divides by w (if w != 1)
Vector3 Vector3::TransformWithPerspDiv(const Vector3& vec, const Matrix4& mat, float w)
{
    Vector3 retVal;
    retVal.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[1][0] +
               vec.z * mat.mat[2][0] + w * mat.mat[3][0];
    retVal.y = vec.x * mat.mat[0][1] + vec.y * mat.mat[1][1] +
               vec.z * mat.mat[2][1] + w * mat.mat[3][1];
    retVal.z = vec.x * mat.mat[0][2] + vec.y * mat.mat[1][2] +
               vec.z * mat.mat[2][2] + w * mat.mat[3][2];
    float transformedW = vec.x * mat.mat[0][3] + vec.y * mat.mat[1][3] +
                         vec.z * mat.mat[2][3] + w * mat.mat[3][3];

    if (!Math::NearZero(Math::Abs(transformedW)))
    {
        transformedW = 1.0f / transformedW;
        retVal *= transformedW;
    }

    return retVal;
}

// Create basic LU decomposition to invert matrix
void Matrix4::Invert()
{
    // Create the identity matrix
    float inv[4][4];
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (i == j)
            {
                inv[i][j] = 1.0f;
            }
            else
            {
                inv[i][j] = 0.0f;
            }
        }
    }

    // Forward elimination
    for (int i = 0; i < 4; i++)
    {
        int pivot = i;
        float pivotSize = mat[i][i];

        if (pivotSize < 0)
        {
            pivotSize = -pivotSize;
        }

        for (int j = i + 1; j < 4; j++)
        {
            float temp = mat[j][i];

            if (temp < 0)
            {
                temp = -temp;
            }

            if (temp > pivotSize)
            {
                pivot = j;
                pivotSize = temp;
            }
        }

        if (Math::NearZero(pivotSize))
        {
            // Cannot invert singular matrix
            return;
        }

        if (pivot != i)
        {
            // Swap rows
            for (int j = 0; j < 4; j++)
            {
                float temp;

                temp = mat[i][j];
                mat[i][j] = mat[pivot][j];
                mat[pivot][j] = temp;

                temp = inv[i][j];
                inv[i][j] = inv[pivot][j];
                inv[pivot][j] = temp;
            }
        }

        // Multiply by 1 / pivot
        float oneOverPivot = 1.0f / mat[i][i];
        for (int j = 0; j < 4; j++)
        {
            mat[i][j] *= oneOverPivot;
            inv[i][j] *= oneOverPivot;
        }

        // Eliminate column
        for (int j = 0; j < 4; j++)
        {
            if (j != i)
            {
                float oneOverS = mat[j][i];
                for (int k = 0; k < 4; k++)
                {
                    mat[j][k] -= oneOverS * mat[i][k];
                    inv[j][k] -= oneOverS * inv[i][k];
                }
            }
        }
    }

    // Copy the inverse matrix over the original
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            mat[i][j] = inv[i][j];
        }
    }
}
