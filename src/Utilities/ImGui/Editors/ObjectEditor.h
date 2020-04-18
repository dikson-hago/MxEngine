// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistributionand use in source and binary forms, with or without
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

#include "Utilities/ImGui/ImGuiBase.h"
#include "Core/Application/Application.h"
#include "Utilities/Format/Format.h"

namespace MxEngine::GUI
{
	inline void DrawMaterial(Material& material)
	{
		ImGui::ColorEdit3("ambient color", &material.Ka[0]);
		ImGui::ColorEdit3("diffuse color", &material.Kd[0]);
		ImGui::ColorEdit3("specular color", &material.Ks[0]);
		ImGui::ColorEdit3("emmisive color", &material.Ke[0]);
		ImGui::DragFloat("ambient factor", &material.f_Ka, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("diffuse factor", &material.f_Kd, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("specular exponent", &material.Ns, 1.0f, 1.0f, 512.0f);
		ImGui::DragFloat("transparency", &material.d, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("reflection", &material.reflection, 0.01f, 0.0f, 1.0f);
	}

	inline void DrawTransform(Transform& transform)
	{
		// translation
		auto translation = transform.GetTranslation();
		if (ImGui::InputFloat3("translation", &translation[0]))
			transform.SetTranslation(translation);

		// rotation (euler)
		auto rotation = DegreesVec(transform.GetEulerRotation());
		auto newRotation = rotation;
		if (ImGui::DragFloat("rotate x", &newRotation.x))
			transform.RotateX(newRotation.x - rotation.x);
		if (ImGui::DragFloat("rotate y", &newRotation.y))
			transform.RotateY(newRotation.y - rotation.y);
		if (ImGui::DragFloat("rotate z", &newRotation.z))
			transform.RotateZ(newRotation.z - rotation.z);

		// scale
		auto scale = transform.GetScale();
		if (ImGui::InputFloat3("scale", &scale[0]))
			transform.SetScale(scale);
	}

	inline void DrawObjectEditor()
	{
		auto context = Application::Get();
		for (const auto& pair : context->GetCurrentScene().GetObjectList())
		{
			GUI_TREE_NODE(pair.first.c_str(),
				auto & object = *pair.second;
				ImGui::PushID(pair.first.c_str());

				// toggle object visibility
				bool isDrawn = object.IsDrawable();
				bool dirVecs = false;
				bool instanced = false;
				if (ImGui::Checkbox("drawn", &isDrawn))
					isDrawn ? object.Show() : object.Hide();

				// toggle dir vec input (see below)
				ImGui::SameLine(); ImGui::Checkbox("dir. vecs", &dirVecs);
				// toggle instance editing (see below)
				ImGui::SameLine(); ImGui::Checkbox("instances", &instanced);
				// add delete button
				ImGui::SameLine(); if (ImGui::Button("delete")) context->GetCurrentScene().DestroyObject(pair.first);

				// current texture path
				ImGui::Text((std::string("texture: ") + (object.ObjectTexture ? object.GetTexture().GetPath() : std::string("none"))).c_str());

				auto renderColor = object.GetRenderColor();
				if (ImGui::ColorEdit4("render color", &renderColor[0]))
					object.SetRenderColor(renderColor);

				DrawTransform(object.ObjectTransform);

				ImGui::InputFloat("translate speed", &object.TranslateSpeed);
				ImGui::InputFloat("rotate speed", &object.RotateSpeed);
				ImGui::InputFloat("scale speed", &object.ScaleSpeed);

				// object texture (loads from file)
				static std::string texturePath(128, '\0');
				ImGui::InputText("texture path", texturePath.data(), texturePath.size());
				ImGui::SameLine();
				if (ImGui::Button("update"))
					object.ObjectTexture = context->GetCurrentScene().LoadTexture(
						Format(FMT_STRING("MxRuntimeTex_{0}"), context->GenerateResourceId()),
						texturePath);

				if (object.GetMesh() != nullptr)
				{
					GUI_TREE_NODE("mesh list",
						for (auto& submesh : object.GetMesh()->GetRenderObjects())
						{
							GUI_TREE_NODE(submesh.GetName().c_str(),
								if (submesh.HasMaterial())
								{
									DrawMaterial(submesh.GetMaterial());
								}
							);
						}
					);
				}

				if (instanced)
				{
					if (object.GetInstanceCount() > 0)
					{
						int idx = 0;
						for (auto& instance : object.GetInstances())
						{
							if (ImGui::CollapsingHeader(Format(FMT_STRING("instance #{0}"), idx).c_str()))
							{
								ImGui::PushID(idx);

								DrawTransform(instance.Model);
								bool draw = instance.IsDrawn();
								if (ImGui::Checkbox("visible", &draw))
								{
									if (draw)
										instance.Show();
									else
										instance.Hide();
								}

								ImGui::PopID();
							}
							idx++;
						}
						if (ImGui::Button("instanciate"))
						{
							object.Instanciate();
						}
						ImGui::SameLine();
						if (ImGui::Button("destroy instances"))
						{
							object.DestroyInstances();
						}
					}
					else if (object.GetMesh() != nullptr && object.GetMesh()->RefCounter == 1)
					{
						if (ImGui::Button("make instanced"))
							object.Instanciate();
					}
				}

				if (dirVecs)
				{
					Vector3 forward = object.GetForwardVector();
					Vector3 up = object.GetUpVector();
					Vector3 right = object.GetRightVector();
					if (ImGui::InputFloat3("forward vec", &forward[0]))
						object.SetForwardVector(forward);
					if (ImGui::InputFloat3("up vec", &up[0]))
						object.SetUpVector(up);
					if (ImGui::InputFloat3("right vec", &right[0]))
						object.SetRightVector(right);
				}

				ImGui::PopID();
			);
		}
	}
}