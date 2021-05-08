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

		private: System::Windows::Forms::Label^  label1;
		private: System::Windows::Forms::TextBox^  timeBox;


		private:
			System::ComponentModel::Container ^components;

		public:
			
	private: System::Windows::Forms::RadioButton^  radioButtonAABB;
	public:

	private: System::Windows::Forms::Button^  Render;
	public:

		Bitmap myBitmap_1 = Bitmap::FromFile("..\\Textures\\carpet.jpg");
		Bitmap myBitmap_2 = Bitmap::FromFile("..\\Textures\\bayrak.jpg");
		Bitmap myBitmap_3 = Bitmap::FromFile("..\\Textures\\Mona_Lisa.jpg");

		#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->surfacePictureBox = (gcnew System::Windows::Forms::PictureBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->timeBox = (gcnew System::Windows::Forms::TextBox());
			this->radioButtonAABB = (gcnew System::Windows::Forms::RadioButton());
			this->Render = (gcnew System::Windows::Forms::Button());
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
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(162)));
			this->label1->Location = System::Drawing::Point(604, 486);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(136, 16);
			this->label1->TabIndex = 2;
			this->label1->Text = L"Rendering Time (sn) :";
			// 
			// timeBox
			// 
			this->timeBox->Font = (gcnew System::Drawing::Font(L"Consolas", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(162)));
			this->timeBox->Location = System::Drawing::Point(742, 481);
			this->timeBox->Name = L"timeBox";
			this->timeBox->Size = System::Drawing::Size(68, 26);
			this->timeBox->TabIndex = 3;
			this->timeBox->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// radioButtonAABB
			// 
			this->radioButtonAABB->AutoSize = true;
			this->radioButtonAABB->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(162)));
			this->radioButtonAABB->Location = System::Drawing::Point(14, 479);
			this->radioButtonAABB->Name = L"radioButtonAABB";
			this->radioButtonAABB->Size = System::Drawing::Size(92, 20);
			this->radioButtonAABB->TabIndex = 4;
			this->radioButtonAABB->TabStop = true;
			this->radioButtonAABB->Text = L"AABB Test";
			this->radioButtonAABB->UseVisualStyleBackColor = true;
			// 
			// Render
			// 
			this->Render->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(162)));
			this->Render->Location = System::Drawing::Point(366, 484);
			this->Render->Name = L"Render";
			this->Render->Size = System::Drawing::Size(79, 24);
			this->Render->TabIndex = 1;
			this->Render->Text = L"Render";
			this->Render->UseVisualStyleBackColor = true;
			this->Render->Click += gcnew System::EventHandler(this, &RayTracer::Render_Click);
			// 
			// RayTracer
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(823, 516);
			this->Controls->Add(this->radioButtonAABB);
			this->Controls->Add(this->timeBox);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->Render);
			this->Controls->Add(this->surfacePictureBox);
			this->Name = L"RayTracer";
			this->Text = L"RayTracer : AABB";
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
			//  Rd  \           Rd : gelen ýþýnýn doðrultusu
			//       \			n1 : 1. ortamýn kýrýlma indisi
			// _______\__________ 
			//        | 
			//        |			n2 : 2. ortamýn kýrýlma indisi
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

		#pragma region Texture Mapping

		void TextureMapping(Shape* S, Vertex iPoint)
		{
			switch (S->Tex)
			{
				case 1:
				{
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
					Vertex L	= iPoint - S->V0;
					Vertex Rdu	= (S->V1 - S->V0).Normalize();
					float Su	= L * Rdu;
					float u		= Su / (S->V1 - S->V0).Length();

					Vertex Rdv	= (S->V2 - S->V0).Normalize();
					float Sv	= L * Rdv;
					float v		= Sv / (S->V2 - S->V0).Length();

					int x = (int)(u * 250);
					int y = (int)(v * 167);

					x = x > 249 ? 249 : x;      x = x < 0 ? 0 : x;
					y = y > 166 ? 166 : y;      y = y < 0 ? 0 : y;

					S->ShapeColor = myBitmap_2.GetPixel(x, y);
				}
				break;

				case 4:
				{
					Vertex L	= iPoint - S->V0;
					Vertex Rdu	= (S->V1 - S->V0).Normalize();
					float Su	= L * Rdu;
					float u		= 1 - Su / (S->V1 - S->V0).Length();

					Vertex Rdv	= (S->V2 - S->V0).Normalize();
					float Sv	= L * Rdv;
					float v		= 1 - Sv / (S->V2 - S->V0).Length();

					int x = (int)(u * 250);
					int y = (int)(v * 167);

					x = x > 249 ? 249 : x;      x = x < 0 ? 0 : x;
					y = y > 166 ? 166 : y;      y = y < 0 ? 0 : y;

					S->ShapeColor = myBitmap_2.GetPixel(x, y);
				}
				break;

				case 5:
				{
					Vertex L	= iPoint - S->V0;
					Vertex Rdu	= (S->V1 - S->V0).Normalize();
					float Su	= L * Rdu;
					float u		= Su / (S->V1 - S->V0).Length();

					Vertex Rdv	= (S->V2 - S->V0).Normalize();
					float Sv	= L * Rdv;
					float v		= Sv / (S->V2 - S->V0).Length();

					int x = (int)(u * 2800);
					int y = (int)(v * 4200);

					x = x > 2799 ? 2799 : x;      x = x < 0 ? 0 : x;
					y = y > 4199 ? 4199 : y;      y = y < 0 ? 0 : y;

					S->ShapeColor = myBitmap_3.GetPixel(x, y);
				}
				break;

				case 6:
				{
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

					S->ShapeColor = myBitmap_3.GetPixel(x, y);
				}
				break;
			}
		}
		#pragma endregion

		#pragma region Shadow Test

		bool TestShadow(Shape* Shapes[], Vertex iPoint)
		{
			Vertex light = Vertex(0, 30, 60);

			Vertex toLight = (light - iPoint).Normalize();

			vector<intersections> Intersections;

			for (int i = 0; i < 94; i++)
			{
				float t = Shapes[i]->Intersect(iPoint, toLight);

				if (t > 0.1F)				// 0.1 for avoiding self intersection
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

				if (min_distance < (light - iPoint).Length())
					return true;
				else
					return false;
			}
			else return false;
		}

		#pragma endregion

		#pragma region AABB Test

		float AABB(Vertex Ro, Vertex Rd, Vertex B, float U, float V, float W)
		{
			float tmin = -1000;
			float tmax =  1000;
			Vertex b = B;
			Vertex p = b - Ro;
			float  u = U;
			float  v = V;
			float  w = W;
			Vertex bu(1, 0, 0);
			Vertex bv(0, 1, 0);
			Vertex bw(0, 0, 1);

			float e = bu * p;
			float f = 1 / (bu * Rd);

			if ((float)Math::Abs(f) > 0.00001F)
			{
				float t1 = (e + u) * f;
				float t2 = (e - u) * f;
				if (t1 > t2) { float temp = t1; t1 = t2; t2 = temp; };
				if (t1 > tmin) tmin = t1;
				if (t2 < tmax) tmax = t2;
				if (tmin > tmax) return 0;
				if (tmax < 0) return 0;
			}
			else if (((-e - u) > 0) || ((-e + u) < 0)) return 0;
			/////////

			e = bv * p;
			f = 1 / (bv * Rd);

			if ((float)Math::Abs(f) > 0.00001F)
			{
				float t1 = (e + v) * f;
				float t2 = (e - v) * f;
				if (t1 > t2) { float temp = t1; t1 = t2; t2 = temp; };
				if (t1 > tmin) tmin = t1;
				if (t2 < tmax) tmax = t2;
				if (tmin > tmax) return 0;
				if (tmax < 0) return 0;
			}
			else if (((-e - v) > 0) || ((-e + v) < 0)) return 0;
			//////////

			e = bw * p;
			f = 1 / (bw * Rd);

			if ((float)Math::Abs(f) > 0.00001F)
			{
				float t1 = (e + w) * f;
				float t2 = (e - w) * f;
				if (t1 > t2) { float temp = t1; t1 = t2; t2 = temp; };
				if (t1 > tmin) tmin = t1;
				if (t2 < tmax) tmax = t2;
				if (tmin > tmax) return 0;
				if (tmax < 0) return 0;
			}
			else if (((-e - w) > 0) || ((-e + w) < 0)) return 0;
			////////

			if (tmin > 0) return tmin;
			else return tmax;

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

			if (radioButtonAABB->Checked)
			{
				if (AABB(Ro, Rd, Vertex(-25, -20, 70), 15, 20, 30) > 0.0)
				{
					for (int i = 16; i < 94; i++)
					{
						float t = Shapes[i]->Intersect(Ro, Rd);
						if (t > 0.1F)				// 0.1 for avoiding self intersection at reflection
						{
							intersection.distance = t;
							intersection.indice   = i;

							Intersections.push_back(intersection);
						}
					}
				}
			}
			else
			{
				for (int i = 16; i < 94; i++)
				{
					float t = Shapes[i]->Intersect(Ro, Rd);
					if (t > 0.1F)								
					{
						intersection.distance = t;
						intersection.indice   = i;

						Intersections.push_back(intersection);
					}
				}
			}

			for (int i = 0; i < 16; i++)
			{
				float t = Shapes[i]->Intersect(Ro, Rd);
				if (t > 0.1F)								
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

				//if (TestShadow(Shapes, iPoint))
				//	return ShadingModel(S, Color::Black, Color::Black, Color::Black, Color::Black, Color::Black, S->Ambient, S->Dif, S->Spec, S->Refl, S->Trans, S->Refrac);

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

			//Flag
			Triangle T1(Vertex(-30, 30, 119), Vertex(30, 30, 119), Vertex(-30, -10, 119), 0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 3);
			Triangle T2(Vertex(30, -10, 119), Vertex(-30, -10, 119), Vertex(30, 30, 119), 0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 4);

			// Mona Lisa (Right Wall)
			Triangle T3( Vertex(  59,  30,   105),  Vertex( 59,  30,  75),  Vertex( 59, -15,  105),   0.3,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 5 );
			Triangle T4( Vertex(  59, -15,    75),  Vertex( 59, -15, 105),  Vertex( 59,  30,   75),   0.3,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 6 );

     
			// ODA
			Triangle T5( Vertex(  60, -40,  120),    Vertex(-60, -40, 120),    Vertex(-60,  40, 120),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0); // Öndeki Duvar 
			Triangle T6( Vertex(  60, -40,  120),    Vertex(-60,  40, 120),    Vertex( 60,  40, 120),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0);
			Triangle T7( Vertex(  60, -40,    0),    Vertex(-60, -40,   0),    Vertex( 60, -40, 120),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 2); // Zemin
			Triangle T8( Vertex( -60, -40,  120),    Vertex( 60, -40, 120),    Vertex(-60, -40,   0),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 1); 
			Triangle T9( Vertex(  60,  40,  120),    Vertex(-60,  40,   0),    Vertex( 60,  40,   0),   0.7, 0.5, 0.0, 0.0, 0.0, 0.0, Color::White, 0); // Tavan 
			Triangle T10( Vertex( 60,  40,  120),    Vertex(-60,  40, 120),    Vertex(-60,  40,   0),   0.7, 0.5, 0.0, 0.0, 0.0, 0.0, Color::White, 0);
			Triangle T11( Vertex( 60,  40,  120),    Vertex( 60,  40,   0),    Vertex( 60, -40,   0),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0); // Saðdaki Duvar 
			Triangle T12( Vertex( 60,  40,  120),    Vertex( 60, -40,   0),    Vertex( 60, -40, 120),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0);
			Triangle T13( Vertex(-60,  40,  120),    Vertex(-60, -40,   0),    Vertex(-60,  40,   0),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0); // Soldaki Duvar 
			Triangle T14( Vertex(-60,  40,  120),    Vertex(-60, -40, 120),    Vertex(-60, -40,   0),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0);
			Triangle T15( Vertex( 60, -40,    0),    Vertex(-60,  40,   0),    Vertex(-60, -40,   0),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0); // Arkadaki Duvar 
			Triangle T16( Vertex( 60, -40,    0),    Vertex( 60,  40,   0),    Vertex(-60,  40,   0),   0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0);

			// MASA
			Triangle T17( Vertex(   -10,  -20,  100),    Vertex(-10,  -20,   40),     Vertex(-40,  -20,   40),   0.3,  0.4, 0.0, 0.4, 0.0, 0.0, Color::Brown, 0 );  // Masa üst
			Triangle T18( Vertex(   -40,  -20,  100),    Vertex(-10,  -20,   100),    Vertex( -40,  -20,   40),  0.3,  0.4, 0.0, 0.4, 0.0, 0.0, Color::Brown, 0 );

			Triangle T19( Vertex(   -10,  -21,   40),    Vertex( -10,  -21,  100),    Vertex( -40,  -21,   40),  0.1,  0.4, 0.0, 0.5, 0.0, 0.0, Color::Brown, 0 );  // Masa alt
			Triangle T20( Vertex(   -10,  -21,   100),   Vertex( -40,  -21,  100),    Vertex( -40,  -21,   40),  0.1,  0.4, 0.0, 0.5, 0.0, 0.0, Color::Brown, 0 );


			Triangle T21( Vertex(   -10,  -20,  40),     Vertex( -10,  -21,   40),    Vertex( -40,  -21,   40),  0.1,  0.4, 0.0, 0.5, 0.0, 0.0, Color::Brown, 0 );  // MASA YANLAR
			Triangle T22( Vertex(   -40,  -20,  40),     Vertex( -10,  -20,   40),    Vertex( -40,  -21,   40),  0.1,  0.4, 0.0, 0.5, 0.0, 0.0, Color::Brown, 0 );

			Triangle T23( Vertex(   -10,  -21,  100),    Vertex( -10,  -20,  100),    Vertex( -40,  -21,   100),  0.1,  0.4, 0.0, 0.5, 0.0, 0.0, Color::Brown, 0 );
			Triangle T24( Vertex(   -10,  -20,  100),    Vertex( -40,  -20,  100),    Vertex( -40,  -21,   100),  0.1,  0.4, 0.0, 0.5, 0.0, 0.0, Color::Brown, 0 );

			Triangle T25( Vertex(   -10,  -20,  100),    Vertex( -10,  -21,   100),   Vertex( -10,  -21,   40),  0.1,  0.4, 0.0, 0.5, 0.0, 0.0, Color::Brown, 0 );
			Triangle T26( Vertex(   -10,  -20,   40),    Vertex( -10,  -20,   100),   Vertex( -10,  -21,   40),  0.1,  0.4, 0.0, 0.5, 0.0, 0.0, Color::Brown, 0 );

			Triangle T27( Vertex( -40,  -21,   100),     Vertex(  -40,  -20,  100),   Vertex( -40,  -21,   40),  0.1,  0.4, 0.0, 0.5, 0.0, 0.0, Color::Brown, 0 );
			Triangle T28( Vertex( -40,  -20,   100),     Vertex(  -40,  -20,   40),   Vertex( -40,  -21,   40),  0.1,  0.4, 0.0, 0.5, 0.0, 0.0, Color::Brown, 0 );

			Triangle T29( Vertex(   -15,  -20,   50),    Vertex( -15,  -40,  50),     Vertex( -40,  -40,   50),  0.4,  0.6, 0.0, 0.0, 0.0, 0.0, Color::Brown, 0 );  // MASA BACAKLAR
			Triangle T30( Vertex(   -40,  -20,   50),    Vertex( -15,  -20,   50),    Vertex( -40,  -40,   50),  0.4,  0.6, 0.0, 0.0, 0.0, 0.0, Color::Brown, 0 );

			Triangle T31( Vertex(   -15,  -20,   52),    Vertex( -15,  -40,   52),    Vertex( -15,  -40,   50),  0.4,  0.6, 0.0, 0.0, 0.0, 0.0, Color::Brown, 0 );
			Triangle T32( Vertex(   -15,  -20,   50),    Vertex( -15,  -20,   52),    Vertex( -15,  -40,   50),  0.4,  0.6, 0.0, 0.0, 0.0, 0.0, Color::Brown, 0 );

			Triangle T33( Vertex(   -15,  -40,   52),    Vertex(-15,  -20,   52),     Vertex( -40,  -40,   52),  0.4,  0.6, 0.0, 0.0, 0.0, 0.0, Color::Brown, 0 );
			Triangle T34( Vertex(   -15,  -20,   52),    Vertex(-40,  -20,   52),     Vertex( -40,  -40,   52),  0.4,  0.6, 0.0, 0.0, 0.0, 0.0, Color::Brown, 0 );


			Triangle T35( Vertex(   -15,  -20,   90),    Vertex(-15,  -40,  90),      Vertex(-40,  -40,   90),  0.4,  0.6, 0.0, 0.0, 0.0, 0.0, Color::Brown, 0 );
			Triangle T36( Vertex(   -40,  -20,   90),    Vertex(-15,  -20,  90),      Vertex(-40,  -40,   90),  0.4,  0.6, 0.0, 0.0, 0.0, 0.0, Color::Brown, 0 );

			Triangle T37( Vertex(   -15,  -20,   92),    Vertex(-15,  -40,  92),      Vertex(-15,  -40,   90),  0.4,  0.6, 0.0, 0.0, 0.0, 0.0, Color::Brown, 0 );
			Triangle T38( Vertex(   -15,  -20,   90),    Vertex(-15,  -20,  92),      Vertex(-15,  -40,   90),  0.4,  0.6, 0.0, 0.0, 0.0, 0.0, Color::Brown, 0 );

			Triangle T39( Vertex(   -15,  -40,   92),    Vertex(-15,  -20,   92),     Vertex(-40,  -40,   92),  0.4,  0.6, 0.0, 0.0, 0.0, 0.0, Color::Brown, 0 );
			Triangle T40( Vertex(   -15,  -20,   92),    Vertex(-40,  -20,   92),     Vertex(-40,  -40,   92),  0.4,  0.6, 0.0, 0.0, 0.0, 0.0, Color::Brown, 0 );

			//		LCD MONITOR
			Triangle T41( Vertex(   -25.9,  -6.5,    72.5),  Vertex(-25.9,    -15.5,    72.5),  Vertex(-25.9,    -15.5,    57.5),  0.2,  0.2, 0.0, 0.6, 0.0, 0.0, Color::Blue, 0 );  // LCD Ekran
			Triangle T42( Vertex(   -25.9,  -6.5,    57.5),  Vertex(-25.9,    -6.5,     72.5),  Vertex(-25.9,    -15.5,    57.5),  0.2,  0.2, 0.0, 0.6, 0.0, 0.0, Color::Blue, 0 );


			Triangle T43( Vertex(   -26,  -6,    73),  Vertex(-26,    -6,    57),  Vertex(-27,     -6,    57),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );  //  LCD Monitor
			Triangle T44( Vertex(   -27,  -6,    73),  Vertex(-26,    -6,    73),  Vertex(-27,     -6,    57),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T45( Vertex(   -26,  -6,    57),  Vertex(-26,   -16,    57),  Vertex(-27,    -16,    57),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T46( Vertex(   -27,  -6,    57),  Vertex(-26,    -6,    57),  Vertex(-27,    -16,    57),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T47( Vertex(   -26, -16,    73),  Vertex(-26,    -6,    73),  Vertex(-27,    -16,    73),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T48( Vertex(   -26,  -6,    73),  Vertex(-27,    -6,    73),  Vertex(-27,    -16,    73),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T49( Vertex(   -26,  -6,    73),  Vertex(-26,   -16,    73),  Vertex(-26,    -16,    57),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T50( Vertex(   -26,  -6,    57),  Vertex(-26,    -6,    73),  Vertex(-26,    -16,    57),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T51( Vertex(   -27, -16,    73),  Vertex(-27,    -6,    73),  Vertex(-27,    -16,    57),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T52( Vertex(   -27,  -6,    73),  Vertex(-27,    -6,    57),  Vertex(-27,    -16,    57),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );


			Triangle T53( Vertex(   -26,  -16,    66),  Vertex(-26,    -16,    64),  Vertex(-27,    -16,    64),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );  // LCD Monitor Altlýk
			Triangle T54( Vertex(   -27,  -16,    66),  Vertex(-26,    -16,    66),  Vertex(-27,    -16,    64),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T55( Vertex(   -26,  -16,    64),  Vertex(-26,    -20,    64),  Vertex(-27,    -20,    64),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T56( Vertex(   -27,  -16,    64),  Vertex(-26,    -16,    64),  Vertex(-27,    -20,    64),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T57( Vertex(   -26,  -20,    66),  Vertex(-26,    -16,    66),  Vertex(-27,    -20,    66),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T58( Vertex(   -26,  -16,    66),  Vertex(-27,    -16,    66),  Vertex(-27,    -20,    66),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T59( Vertex(   -26,  -16,    66),  Vertex(-26,    -20,    66),  Vertex(-26,    -20,    64),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T60( Vertex(   -26,  -16,    64),  Vertex(-26,    -16,    66),  Vertex(-26,    -20,    64),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T61( Vertex(   -27,  -20,    66),  Vertex(-27,    -16,    66),  Vertex(-27,    -20,    64),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T62( Vertex(   -27,  -16,    66),  Vertex(-27,    -16,    64),  Vertex(-27,    -20,    64),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );

			Triangle T63( Vertex(   -26,  -19,   66),    Vertex(-23,   -20,   66),    Vertex(-26,   -19,   64),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T64( Vertex(   -23,  -20,   66),    Vertex(-23,   -20,   64),    Vertex(-26,   -19,   64),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );

			Triangle T65( Vertex(   -30,  -20,   66),    Vertex(-27,   -19,   66),    Vertex(-27,   -19,   64),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T66( Vertex(   -30,  -20,   64),    Vertex(-30,   -20,   66),    Vertex(-27,   -19,   64),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );

			Triangle T67( Vertex(   -26,  -19,   64),    Vertex(-26,   -20,   61),    Vertex(-27,   -20,   61),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T68( Vertex(   -27,  -19,   64),    Vertex(-26,   -19,   64),    Vertex(-27,   -20,   61),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );

			Triangle T69( Vertex(   -26,  -20,   69),    Vertex(-26,   -19,   66),    Vertex(-27,   -20,   69),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T70( Vertex(   -26,  -19,   66),    Vertex(-27,   -19,   66),    Vertex(-27,   -20,   69),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );

			Triangle T71( Vertex(   -26,  -19,   64),    Vertex(-23,   -20,   64),    Vertex(-26,   -20,   61),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T72( Vertex(   -26,  -19,   66),    Vertex(-26,   -20,   69),    Vertex(-23,   -20,   66),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T73( Vertex(   -27,  -19,   64),    Vertex(-27,   -20,   61),    Vertex(-30,   -20,   64),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T74( Vertex(   -27,  -19,   66),    Vertex(-30,   -20,   66),    Vertex(-27,   -20,   69),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );

			// KLAVYE
			Triangle T75( Vertex(   -14,  -19.5,   70),   Vertex(-14,   -19.5,   60),   Vertex(-18,   -19,   60),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );  
			Triangle T76( Vertex(   -18,    -19,   70),   Vertex(-14,   -19.5,   70),   Vertex(-18,   -19,   60),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T77( Vertex(   -14,  -19.5,   60),   Vertex(-14,     -20,   60),   Vertex(-18,   -20,   60),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T78( Vertex(   -18,    -19,   60),   Vertex(-14,   -19.5,   60),   Vertex(-18,   -20,   60),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T79( Vertex(   -14,    -20,   70),   Vertex(-14,   -19.5,   70),   Vertex(-18,   -20,   70),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T80( Vertex(   -14,  -19.5,   70),   Vertex(-18,     -19,   70),   Vertex(-18,   -20,   70),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T81( Vertex(   -14,  -19.5,   70),   Vertex(-14,     -20,   70),   Vertex(-14,   -20,   60),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T82( Vertex(   -14,  -19.5,   60),   Vertex(-14,   -19.5,   70),   Vertex(-14,   -20,   60),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T83( Vertex(   -18,    -20,   70),   Vertex(-18,     -19,   70),   Vertex(-18,   -20,   60),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T84( Vertex(   -18,    -19,   70),   Vertex(-18,     -19,   60),   Vertex(-18,   -20,   60),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );

			// KASA
			Triangle T85( Vertex(   -15,  -6,    80),  Vertex(-15,    -6,    75),  Vertex(-30,     -6,    75),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );  
			Triangle T86( Vertex(   -30,  -6,    80),  Vertex(-15,    -6,    80),  Vertex(-30,     -6,    75),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T87( Vertex(   -15,  -6,    75),  Vertex(-15,   -20,    75),  Vertex(-30,    -20,    75),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T88( Vertex(   -30,  -6,    75),  Vertex(-15,    -6,    75),  Vertex(-30,    -20,    75),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T89( Vertex(   -15, -20,    80),  Vertex(-15,    -6,    80),  Vertex(-30,    -20,    80),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T90( Vertex(   -15,  -6,    80),  Vertex(-30,    -6,    80),  Vertex(-30,    -20,    80),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T91( Vertex(   -15,  -6,    80),  Vertex(-15,   -20,    80),  Vertex(-15,    -20,    75),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T92( Vertex(   -15,  -6,    75),  Vertex(-15,    -6,    80),  Vertex(-15,    -20,    75),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T93( Vertex(   -30, -20,    80),  Vertex(-30,    -6,    80),  Vertex(-30,    -20,    75),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );
			Triangle T94( Vertex(   -30,  -6,    80),  Vertex(-30,    -6,    75),  Vertex(-30,    -20,    75),  0.5,  0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 0 );

			Shape* Shapes[94] = 
			{ 
				&T1, &T2, &T3, &T4, &T5, &T6, &T7, &T8, &T9, &T10, &T11, &T12, &T13, &T14, &T15, &T16, &T17, &T18, &T19, &T20, &T21, &T22, &T23, &T24, &T25, &T26, &T27, &T28, &T29, &T30,
				&T31, &T32, &T33, &T34, &T35, &T36, &T37, &T38, &T39, &T40, &T41, &T42, &T43, &T44, &T45, &T46, &T47, &T48, &T49, &T50, &T51, &T52, &T53, &T54, &T55, &T56, &T57, &T58, &T59, &T60,
				&T61, &T62, &T63, &T64, &T65, &T66, &T67, &T68, &T69, &T70, &T71, &T72, &T73, &T74, &T75, &T76, &T77, &T78, &T79, &T80, &T81, &T82, &T83, &T84, &T85, &T86, &T87, &T88, &T89, &T90, &T91, &T92, &T93, &T94
			};

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

			DateTime date;
			String^ filename = date.Now.Year.ToString() + date.Now.Month.ToString() + date.Now.Day.ToString() +
				date.Now.Hour.ToString() + date.Now.Minute.ToString() + date.Now.Second.ToString() + ".jpg";
			surface->Save("image/" + filename);

			radioButtonAABB->Checked = false;
		}
	};
}