#pragma once

#include "Shape.h"
#include "Triangle.h"
#include "Sphere.h"
#include <vector>

namespace RayTracer 
{
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace std;

	struct intersections
	{
		float distance;
		int   indice;
	} intersection;

	public ref class RayTracer : public System::Windows::Forms::Form
	{
		public:
			RayTracer(void)
			{
				InitializeComponent();
			}

		protected:
			~RayTracer()
			{
				if (components)
				{
					delete components;
				}
			}

		private: System::Windows::Forms::PictureBox^  surfacePictureBox;
		protected:
		private: System::Windows::Forms::Button^  Render;
		private: System::Windows::Forms::Label^  label1;
		private: System::Windows::Forms::TextBox^  timeBox;


		private:
			System::ComponentModel::Container ^components;

		#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->surfacePictureBox = (gcnew System::Windows::Forms::PictureBox());
			this->Render = (gcnew System::Windows::Forms::Button());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->timeBox = (gcnew System::Windows::Forms::TextBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->surfacePictureBox))->BeginInit();
			this->SuspendLayout();
			// 
			// surfacePictureBox
			// 
			this->surfacePictureBox->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->surfacePictureBox->Location = System::Drawing::Point(12, 12);
			this->surfacePictureBox->Name = L"surfacePictureBox";
			this->surfacePictureBox->Size = System::Drawing::Size(800, 450);
			this->surfacePictureBox->TabIndex = 0;
			this->surfacePictureBox->TabStop = false;
			// 
			// Render
			// 
			this->Render->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(162)));
			this->Render->Location = System::Drawing::Point(376, 475);
			this->Render->Name = L"Render";
			this->Render->Size = System::Drawing::Size(79, 24);
			this->Render->TabIndex = 1;
			this->Render->Text = L"Render";
			this->Render->UseVisualStyleBackColor = true;
			this->Render->Click += gcnew System::EventHandler(this, &RayTracer::Render_Click);
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(162)));
			this->label1->Location = System::Drawing::Point(604, 480);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(136, 16);
			this->label1->TabIndex = 2;
			this->label1->Text = L"Rendering Time (sn) :";
			// 
			// timeBox
			// 
			this->timeBox->Font = (gcnew System::Drawing::Font(L"Consolas", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(162)));
			this->timeBox->Location = System::Drawing::Point(742, 475);
			this->timeBox->Name = L"timeBox";
			this->timeBox->Size = System::Drawing::Size(68, 26);
			this->timeBox->TabIndex = 3;
			this->timeBox->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// RayTracer
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(823, 512);
			this->Controls->Add(this->timeBox);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->Render);
			this->Controls->Add(this->surfacePictureBox);
			this->Name = L"RayTracer";
			this->Text = L"RayTracer : Reflection Triangle Sample";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->surfacePictureBox))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
		#pragma endregion
		
		#pragma region ShadeDiffuse ShadeSpecular ShadingModel

		Color ShadeDiffuse(Shape* S, Vertex iPoint)
		{
			Vertex light = Vertex(0, 30, 60);

			Vertex toLight = (light - iPoint).Normalize();
			Vertex normal = S->NormalAt(iPoint).Normalize();
			Color c = S->ShapeColor;

			float diffuseKatsayi = normal * toLight;
			if (diffuseKatsayi < 0.0f) return Color::Black;

			float r = 0, g = 0, b = 0;
			r += diffuseKatsayi * c.R;
			g += diffuseKatsayi * c.G;
			b += diffuseKatsayi * c.B;

			r = r > 255 ? 255 : r; r = r < 0 ? 0 : r;
			g = g > 255 ? 255 : g; g = g < 0 ? 0 : g;
			b = b > 255 ? 255 : b; b = b < 0 ? 0 : b;

			return Color::FromArgb((int)r, (int)g, (int)b);
		}

		Color ShadeSpecular(Shape* S, Vertex iPoint, Vertex camera)
		{
			Vertex light = Vertex(0, 30, 60);

			Vertex fromLight = (iPoint - light).Normalize();
			Vertex normal = S->NormalAt(iPoint).Normalize();
			Vertex toCamera = (camera - iPoint).Normalize();

			Color Lamba = Color::White;
			Vertex reflected = (fromLight - 2 * (normal * fromLight) * normal).Normalize();
			float dotProduct = reflected * toCamera;
			if (dotProduct < 0.0f) return Color::Black;

			float specularKatsayi = (float)Math::Pow((double)(dotProduct), (double)8);

			float r = 0, g = 0, b = 0;
			r += specularKatsayi * Lamba.R;
			g += specularKatsayi * Lamba.G;
			b += specularKatsayi * Lamba.B;

			r = r > 255 ? 255 : r; r = r < 0 ? 0 : r;
			g = g > 255 ? 255 : g; g = g < 0 ? 0 : g;
			b = b > 255 ? 255 : b; b = b < 0 ? 0 : b;

			return Color::FromArgb((int)r, (int)g, (int)b);
		}

		Color ShadingModel(Shape* S, Color diffuseColor, Color specularColor, Color reflectedColor, float amb, float dif, float spec, float refl)
		{
			Color ambientcolor = S->ShapeColor;

			int r = Math::Min(255, (int)(amb * ambientcolor.R + dif * diffuseColor.R + spec * specularColor.R + refl * reflectedColor.R));
			int g = Math::Min(255, (int)(amb * ambientcolor.G + dif * diffuseColor.G + spec * specularColor.G + refl * reflectedColor.G));
			int b = Math::Min(255, (int)(amb * ambientcolor.B + dif * diffuseColor.B + spec * specularColor.B + refl * reflectedColor.B));

			return Color::FromArgb(r, g, b);
		}

		#pragma endregion

		#pragma region Calculate Reflection

		Vertex CalculateReflection(Shape* S, Vertex iPoint, Vertex Rd)
		{
			Vertex normal = S->NormalAt(iPoint).Normalize();

			String^ message = "Normal = ( " + normal.X.ToString() + ", " + normal.Y.ToString() + ", " + normal.Z.ToString() + " )";
			MessageBox::Show(message);

			Vertex refDirection = (Rd - 2 * (normal * Rd) * normal).Normalize();

			message = "Reflected Direction = ( " + refDirection.X.ToString() + ", " + refDirection.Y.ToString() + ", " + refDirection.Z.ToString() + " )";
			MessageBox::Show(message);

			return refDirection;
		}

		#pragma endregion

		Color TraceRay(Vertex Ro, Vertex Rd, Shape* Shapes[], Vertex camera, int depth, Shape* prevShape)
		{
			if (depth > 4)
			{
				return prevShape->ShapeColor;
				//return Color::Black;
			}

			vector<intersections> Intersections;

			for (int i = 0; i < 3; i++)
			{
				float t = Shapes[i]->Intersect(Ro, Rd);

				if (t > 0.1F)						// 0.1 is a bias value for avoiding self intersection
				{
					intersection.distance = t;
					intersection.indice   = i;

					Intersections.push_back(intersection);

					String^ name;
					if (i == 0) name = "Kirmizi renkli 1. üçgene olan ";
					else if (i == 1) name = "Yesil renkli 2. üçgene olan ";
					else if (i == 2) name = "Mavi renkli 3. üçgene olan ";
					String^ message = name + "\nt uzakligi = " + t;
					MessageBox::Show(message);
				}
			}

			if (Intersections.size() > 0)
			{
				float min_distance = FLT_MAX;
				int   min_indis = -1;

				for (int i = 0; i < Intersections.size(); i++)
				{
					if (Intersections[i].distance < min_distance)
					{
						min_indis    = Intersections[i].indice;
						min_distance = Intersections[i].distance;
					}
				}

				Vertex iPoint = Ro + min_distance * Rd;		

				String^ name;
				if (min_indis == 0) name = "Kirmizi renkli 1. üçgen üzerindeki "; 
				else if (min_indis == 1) name = "Yesil renkli 2. üçgen üzerindeki ";
				else if (min_indis == 2) name = "Mavi renkli 3. üçgen üzerindeki ";
				String^ message = name + "\niPoint = ( " + iPoint.X.ToString() + ", " + iPoint.Y.ToString() + ", " + iPoint.Z.ToString() + " )";
				MessageBox::Show(message);
		
				Shape* S                 = Shapes[min_indis];

				Color reflectedColor;
				if (S->Refl != 0.0F)
				{
					Vertex reflectedDirection = CalculateReflection(S, iPoint, Rd);
					reflectedColor = TraceRay(iPoint, reflectedDirection, Shapes, camera, depth + 1, S);
				}
				
				Color diffuseColor  = ShadeDiffuse (S, iPoint);
				Color specularColor = ShadeSpecular(S, iPoint, camera);

				return ShadingModel(S, diffuseColor, specularColor, reflectedColor, S->Ambient, S->Dif, S->Spec, S->Refl);
			}

			return Color::Black;
		}

		private: System::Void Render_Click(System::Object^  sender, System::EventArgs^  e) 
		{
			DateTime startTime; 
			startTime = startTime.Now;

			Bitmap^ surface = gcnew Bitmap(800, 450);
			surfacePictureBox->Image = surface;
			
			// Işın Red ve Green üçgenlerden cameraya yakın olanından yansıyıp Blue üçgenle kesişiyor.
			Triangle T1(Vertex(-60, 30, 120), Vertex( 40,  30, 60), Vertex(-40, -30, 120), 0.2F, 0.2F, 0.0F, 0.6F, Color::Red);
			Triangle T2(Vertex( 40, 30,  40), Vertex( 40, -30, 40), Vertex(-40, -30, 100), 0.2F, 0.2F, 0.0F, 0.6F, Color::Green);
			Triangle T3(Vertex(-24, 20,  70), Vertex(-24, -20, 85), Vertex(-24, -20,  55), 0.2F, 0.8F, 0.0F, 0.0F, Color::Blue);

			Shape* Shapes[3] = { &T1, &T2, &T3 };

			Vertex camera = Vertex(0, 0, 0);

			Vertex Rd(0, 0, 1);

			Color  c = TraceRay(camera, Rd, Shapes, camera, 0, NULL);

			//for (int y = 0; y < 450; y++)
			//{
			//	for (int x = 0; x < 800; x++)
			//	{
			//		Vertex pixel = Vertex(16 * x / 799.0F - 8, 4.5 - y * 9 / 449.0F, 10);
			//		Vertex Rd = (pixel - camera).Normalize();
			//		Color c = TraceRay(camera, Rd, Shapes, camera, 0, NULL);
			//		surface->SetPixel(x, y, c);
			//	}
			//	surfacePictureBox->Refresh();
			//}

			DateTime endTime;
			endTime = endTime.Now;
			TimeSpan deltaTime = endTime - startTime;
			timeBox->Text = (Math::Round(100 * deltaTime.TotalSeconds) / 100.0F).ToString();

			//DateTime date;
			//String^ filename = date.Now.Year.ToString() + date.Now.Month.ToString() + date.Now.Day.ToString() +
			//	date.Now.Hour.ToString() + date.Now.Minute.ToString() + date.Now.Second.ToString() + ".jpg";
			//surface->Save("image/" + filename);
		}
	};
}