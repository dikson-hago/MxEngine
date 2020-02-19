// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistributionand use in sourceand binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// 
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditionsand the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditionsand the following disclaimer in the documentation
// and /or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

namespace MxEngine
{
    struct IMovable
    {
        virtual IMovable& Translate(float x, float y, float z) = 0;
        virtual IMovable& TranslateForward(float dist) = 0;
        virtual IMovable& TranslateRight(float dist) = 0;
        virtual IMovable& TranslateUp(float dist) = 0;
        virtual IMovable& Rotate(float horz, float vert) = 0;

        virtual void SetForwardVector(const Vector3& forward) = 0;
        virtual void SetUpVector(const Vector3& up) = 0;
        virtual void SetRightVector(const Vector3& right) = 0;

        virtual const Vector3& GetForwardVector() const = 0;
        virtual const Vector3& GetUpVector() const = 0;
        virtual const Vector3& GetRightVector() const = 0;

        virtual ~IMovable() = default;
    };
}