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

		public:
			Bitmap myBitmap_1 = Bitmap::FromFile("..\\Textures\\carpet.jpg");
			Bitmap myBitmap_2 = Bitmap::FromFile("..\\Textures\\Mona_Lisa.jpg");

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
			this->label1->Location = System::Drawing::Point(604, 478);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(136, 16);
			this->label1->TabIndex = 2;
			this->label1->Text = L"Rendering Time (sn) :";
			// 
			// timeBox
			// 
			this->timeBox->Font = (gcnew System::Drawing::Font(L"Consolas", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(162)));
			this->timeBox->Location = System::Drawing::Point(742, 473);
			this->timeBox->Name = L"timeBox";
			this->timeBox->Size = System::Drawing::Size(68, 26);
			this->timeBox->TabIndex = 3;
			this->timeBox->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// RayTracer
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(823, 509);
			this->Controls->Add(this->timeBox);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->Render);
			this->Controls->Add(this->surfacePictureBox);
			this->Name = L"RayTracer";
			this->Text = L"Texture Mapping iPoint Extended";
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

		Color ShadingModel(Shape* S, Color diffuseColor, Color specularColor, Color reflectedColor, Color transmittedColor, Color refractedColor, float amb, float dif, float spec, float refl, float trans, float refrac)
		{
			Color ambientcolor = S->ShapeColor;

			int r = Math::Min(255, (int)(amb * ambientcolor.R + dif * diffuseColor.R + spec * specularColor.R + refl * reflectedColor.R + trans * transmittedColor.R + refrac * refractedColor.R));
			int g = Math::Min(255, (int)(amb * ambientcolor.G + dif * diffuseColor.G + spec * specularColor.G + refl * reflectedColor.G + trans * transmittedColor.G + refrac * refractedColor.G));
			int b = Math::Min(255, (int)(amb * ambientcolor.B + dif * diffuseColor.B + spec * specularColor.B + refl * reflectedColor.B + trans * transmittedColor.B + refrac * refractedColor.B));

			return Color::FromArgb(r, g, b);
		}

		#pragma endregion

		#pragma region Calculate Reflection Transmission Refraction

		Vertex CalculateReflection(Shape* S, Vertex iPoint, Vertex Rd)
		{
			Vertex normal = S->NormalAt(iPoint).Normalize();
			return (Rd - 2 * (normal * Rd) * normal).Normalize();
		}

		Vertex CalculateTransmission(Shape* S, Vertex iPoint, Vertex Rd)
		{
			return Rd;
		}

		Vertex CalculateRefraction(Shape* S, Vertex iPoint, Vertex Rd, float n1, float n2)
		{
			// Real-Time Rendering (2nd Edition) page : 246
			//
			//  Rd  \           Rd : gelen ışının doğrultusu
			//       \			n1 : 1. ortamın kırılma indisi
			// _______\__________ 
			//        | 
			//        |			n2 : 2. ortamın kırılma indisi
			//        | 
			//        t			t  : refracted direction
			//
			// r = n1 / n2
			// w = -(Rd*n)r
			// k = sqrt(1+(w-r)(w+r))
			// t = r*Rd + (w-k)n       (refracted direction)

			Vertex normal = S->NormalAt(iPoint).Normalize();
			float       r = n1 / n2;
			float       w = -(Rd * normal) * r;
			float       k = (float)Math::Sqrt((double)(1 + (w - r) * (w + r)));
			return      (r * Rd + normal * (w-k)).Normalize(); // t
		}

		#pragma endregion

		#pragma region Texture Mapping

		void TextureMapping(Shape* S, Vertex iPoint)
		{
			switch (S->Tex)
			{
				case 1:
				{
					// I. ÇÖZÜM (OMER CAKIR)

					Vertex L	= iPoint - S->V0;
					Vertex Rdu	= (S->V1 - S->V0).Normalize();
					float Su	= L * Rdu;
					float u		= Su / (S->V1 - S->V0).Length();

					Vertex Rdv	= (S->V2 - S->V0).Normalize();
					float Sv	= L * Rdv;
					float v		= Sv / (S->V2 - S->V0).Length();

					int x = (int)(u * 680);
					int y = (int)(v * 1024);

					x = x > 679 ? 679 : x;        x = x < 0 ? 0 : x;
					y = y > 1023 ? 1023 : y;      y = y < 0 ? 0 : y;

					S->ShapeColor = myBitmap_1.GetPixel(x, y);
				}
				break;

				case 2:
				{
					// I. ÇÖZÜM (OMER CAKIR)

					Vertex L	= iPoint - S->V0;
					Vertex Rdu	= (S->V1 - S->V0).Normalize();
					float Su	= L * Rdu;
					float u		= 1 - Su / (S->V1 - S->V0).Length();

					Vertex Rdv	= (S->V2 - S->V0).Normalize();
					float Sv	= L * Rdv;
					float v		= 1 - Sv / (S->V2 - S->V0).Length();

					int x = (int)(u * 680);
					int y = (int)(v * 1024);

					x = x > 679 ? 679 : x;        x = x < 0 ? 0 : x;
					y = y > 1023 ? 1023 : y;      y = y < 0 ? 0 : y;

					S->ShapeColor = myBitmap_1.GetPixel(x, y);
				}
				break;				

				case 3:
				{
					// I. ÇÖZÜM (OMER CAKIR)

					//Vertex L	= iPoint - S->V0;
					//Vertex Rdu	= (S->V1 - S->V0).Normalize();
					//float Su	= L * Rdu;
					//float u		= Su / (S->V1 - S->V0).Length();

					//Vertex Rdv	= (S->V2 - S->V0).Normalize();
					//float Sv	= L * Rdv;
					//float v		= Sv / (S->V2 - S->V0).Length();


					// II. ÇÖZÜM
					// http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html

					//Vertex X0_X1 = iPoint - S->V0;
					//Vertex X0_X2 = iPoint - S->V1;
					//Vertex X2_X1 = S->V1  - S->V0;
					//float dv = (X0_X1.CrossProduct(X0_X2)).Length() / X2_X1.Length();
					//float  v = dv / (S->V2 - S->V0).Length();

					//X0_X2 = iPoint - S->V2;
					//X2_X1 = S->V2  - S->V0;
					//float du = (X0_X1.CrossProduct(X0_X2)).Length() / X2_X1.Length();
					//float  u = du / (S->V1 - S->V0).Length();


					// III. ÇÖZÜM
					// Realtime Collision Detection, Page : 47
					// iPoint = S->V0 + u(S->V1 - S->V0) + v(S->V2 - S->V0)  
					// u(S->V1 - S->V0) + v(S->V2 - S->V0) = iPoint - S->V0 
					// u*e0 + v*e1 = e2
					// (u*e0 + v*e1)*e0 = e2*e0
					// (u*e0 + v*e1)*e1 = e2*e1
					// u*(e0*e0) + v*(e1*e0) = e2*e0
					// u*(e0*e1) + v*(e1*e1) = e2*e1
					// u*d00 + v*d01 = d20
					// u*d01 + v*d11 = d21

					//Vertex e0 = S->V1 - S->V0;
					//Vertex e1 = S->V2 - S->V0;
					//Vertex e2 = iPoint - S->V0;
					//float d00 = e0 * e0;
					//float d01 = e0 * e1;
					//float d20 = e2 * e0;
					//float d11 = e1 * e1;
					//float d21 = e2 * e1;
					//float u	= (d20 * d11 - d21 * d01) / (d00 * d11 - d01 * d01);
					//float v = (d20 * d01 - d21 * d00) / (d01 * d01 - d11 * d00);


					// IV. ÇÖZÜM
					// Realtime Collision Detection, Page : 49

					float v = ((S->V1 - S->V0).CrossProduct(iPoint - S->V0)).Length() / ((S->V1 - S->V0).CrossProduct(S->V2 - S->V0)).Length();
					float u = ((S->V2 - S->V0).CrossProduct(iPoint - S->V0)).Length() / ((S->V1 - S->V0).CrossProduct(S->V2 - S->V0)).Length();

					int x = (int)(u * 2800);
					int y = (int)(v * 4200);

					x = x > 2799 ? 2799 : x;      x = x < 0 ? 0 : x;
					y = y > 4199 ? 4199 : y;      y = y < 0 ? 0 : y;

					S->ShapeColor = myBitmap_2.GetPixel(x, y);
				}
				break;

				case 4:
				{
					// I. ÇÖZÜM (OMER CAKIR)

					Vertex L	= iPoint - S->V0;
					Vertex Rdu	= (S->V1 - S->V0).Normalize();
					float Su	= L * Rdu;
					float u		= 1 - Su / (S->V1 - S->V0).Length();

					Vertex Rdv	= (S->V2 - S->V0).Normalize();
					float Sv	= L * Rdv;
					float v		= 1 - Sv / (S->V2 - S->V0).Length();

					int x = (int)(u * 2800);
					int y = (int)(v * 4200);

					x = x > 2799 ? 2799 : x;      x = x < 0 ? 0 : x;
					y = y > 4199 ? 4199 : y;      y = y < 0 ? 0 : y;

					S->ShapeColor = myBitmap_2.GetPixel(x, y);
				}
				break;
			}
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

			for (int i = 0; i < 18; i++)
			{
				float t = Shapes[i]->Intersect(Ro, Rd);

				if (t > 0.1F)						// 0.1 for avoiding self intersection at reflection
				{
					intersection.distance = t;
					intersection.indice   = i;

					Intersections.push_back(intersection);
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
				Shape* S      = Shapes[min_indis];

				TextureMapping(S, iPoint);

				Color reflectedColor;
				if (S->Refl != 0.0F)
				{
					Vertex reflectedDirection = CalculateReflection(S, iPoint, Rd);
					reflectedColor = TraceRay(iPoint, reflectedDirection, Shapes, camera, depth + 1, S);
				}

				Color transmittedColor;
				if (S->Trans != 0.0F)
				{
					Vertex transmittedDirection = CalculateTransmission(S, iPoint, Rd);
					transmittedColor = TraceRay(iPoint, transmittedDirection, Shapes, camera, depth + 1, S);
				}

				Color refractedColor;
				if (S->Refrac != 0.0F)
				{
					Vertex refractedDirection = CalculateRefraction(S, iPoint, Rd, 1.0F, 1.33F);
					refractedColor = TraceRay(iPoint, refractedDirection, Shapes, camera, depth + 1, S);
				}

				Color diffuseColor  = ShadeDiffuse(S, iPoint);
				Color specularColor = ShadeSpecular(S, iPoint, camera);

				return ShadingModel(S, diffuseColor, specularColor, reflectedColor, transmittedColor, refractedColor, S->Ambient, S->Dif, S->Spec, S->Refl, S->Trans, S->Refrac);
			}

			return Color::Black;
		}

		private: System::Void Render_Click(System::Object^  sender, System::EventArgs^  e) 
		{
			DateTime startTime; 
			startTime = startTime.Now;

			Bitmap^ surface = gcnew Bitmap(800, 450);
			surfacePictureBox->Image = surface;

			// Mona Lisa (Left)
			Triangle T1(  Vertex( -60,  35,   96),     Vertex(-28,  35,  120),     Vertex( -60, -25,   96),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 3);
			Triangle T2(  Vertex( -28, -25,  120),     Vertex(-60, -25,   96),     Vertex( -28,  35,  120),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 4);

			// Zemin
			Triangle T3(  Vertex( 60, -25,    0),     Vertex(-60, -25,   0),     Vertex( 60, -25, 120),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 2);
			Triangle T4(  Vertex(-60, -25,  120),     Vertex( 60, -25, 120),     Vertex(-60, -25,   0),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 1); 
        
			// Oda
			Triangle T5(   Vertex( 60, -25,  120),     Vertex(-60, -25, 120),     Vertex(-60,  40, 120),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0); // Öndeki Duvar 
			Triangle T6(   Vertex( 60, -25,  120),     Vertex(-60,  40, 120),     Vertex( 60,  40, 120),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0);
			Triangle T7(   Vertex( 60,  40,  120),     Vertex(-60,  40,   0),     Vertex( 60,  40,   0),   0.7, 0.5, 0.0, 0.0, 0.0, 0.0, Color::White, 0); // Tavan 
			Triangle T8(   Vertex( 60,  40,  120),     Vertex(-60,  40, 120),     Vertex(-60,  40,   0),   0.7, 0.5, 0.0, 0.0, 0.0, 0.0, Color::White, 0);
			Triangle T9(   Vertex( 60,  40,  120),     Vertex( 60,  40,   0),     Vertex( 60, -25,   0),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0); // Sagdaki Duvar 
			Triangle T10(  Vertex( 60,  40,  120),     Vertex( 60, -25,   0),     Vertex( 60, -25, 120),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0);
			Triangle T11(  Vertex(-60,  40,  120),     Vertex(-60, -25,   0),     Vertex(-60,  40,   0),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0); // Soldaki Duvar 
			Triangle T12(  Vertex(-60,  40,  120),     Vertex(-60, -25, 120),     Vertex(-60, -25,   0),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0);
			Triangle T13(  Vertex( 60, -25,    0),     Vertex(-60,  40,   0),     Vertex(-60, -25,   0),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0); // Arkadaki Duvar 
			Triangle T14(  Vertex( 60, -25,    0),     Vertex( 60,  40,   0),     Vertex(-60,  40,   0),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0);
			
			// Mona Lisa (Right)
			Triangle T15(  Vertex( 28,  35,  120),     Vertex( 60,  35,   96),     Vertex( 28, -25,  120),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 3);
			Triangle T16(  Vertex( 60, -25,   96),     Vertex( 28, -25,  120),     Vertex( 60,  35,   96),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 4);

			// Mona Lisa (Middle)
			Triangle T17(  Vertex( 12,   7,  116),     Vertex( 36, -25,   84),     Vertex(-36,   7,   80),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 3);
			Triangle T18(  Vertex(-12, -25,   48),     Vertex(-36,   7,   80),     Vertex( 36, -25,   84),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 4);

			Shape* Shapes[18] = { &T1, &T2, &T3, &T4, &T5, &T6, &T7, &T8, &T9, &T10, &T11, &T12, &T13, &T14, &T15, &T16, &T17, &T18 };

			Vertex camera = Vertex(0, 0, 0);

			for (int y = 0; y < 450; y++)
			{
				for (int x = 0; x < 800; x++)
				{
					Vertex pixel = Vertex(16 * x / 799.0F - 8, 4.5 - y * 9 / 449.0F, 10);
					Vertex Rd = (pixel - camera).Normalize();
					Color c = TraceRay(camera, Rd, Shapes, camera, 0, NULL);
					surface->SetPixel(x, y, c);
				}
				surfacePictureBox->Refresh();
			}

			DateTime endTime;
			endTime = endTime.Now;
			TimeSpan deltaTime = endTime - startTime;
			timeBox->Text = (Math::Round(100 * deltaTime.TotalSeconds) / 100.0F).ToString();

			//DateTime date;
			//String^ filename = date.Now.yar.ToString() + date.Now.Month.ToString() + date.Now.Day.ToString() +
			//	date.Now.Hour.ToString() + date.Now.Minute.ToString() + date.Now.Second.ToString() + ".jpg";
			//surface->Save("image/" + filename);
		}
	};
}