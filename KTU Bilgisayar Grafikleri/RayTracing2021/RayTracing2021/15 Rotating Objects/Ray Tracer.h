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

	Vertex camera( 0, 0,  0 );
	Vertex P0( -8,  4.5, 10 );
	Vertex P1(  8,  4.5, 10 );
	Vertex P2(  8, -4.5, 10 );
	Vertex P3( -8, -4.5, 10 );

	// Front
	Triangle T1 (Vertex(-10,  10,  40), Vertex( 10,  10, 40), Vertex(-10, -10, 40), 0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 1);
	Triangle T2 (Vertex( 10, -10,  40), Vertex(-10, -10, 40), Vertex( 10,  10, 40), 0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 2);

	//Back
	Triangle T3 (Vertex( 10,  10, 60),  Vertex(-10,  10, 60), Vertex( 10, -10, 60), 0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 1);
	Triangle T4 (Vertex(-10, -10, 60),  Vertex( 10, -10, 60), Vertex(-10,  10, 60), 0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 2);

	// Right
	Triangle T5 (Vertex( 10,  10, 40),  Vertex( 10,  10, 60), Vertex( 10, -10, 40), 0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 1);
	Triangle T6 (Vertex( 10, -10, 60),  Vertex( 10, -10, 40), Vertex( 10,  10, 60), 0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 2);

	// Left
	Triangle T7 (Vertex(-10,  10, 60),  Vertex(-10,  10, 40), Vertex(-10, -10, 60), 0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 1);
	Triangle T8 (Vertex(-10, -10, 40),  Vertex(-10, -10, 60), Vertex(-10,  10, 40), 0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 2);

	// Ceil
	Triangle T9 (Vertex(-10,  10, 60),  Vertex( 10,  10, 60), Vertex(-10,  10, 40), 0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 1);
	Triangle T10(Vertex( 10,  10, 40),  Vertex(-10,  10, 40), Vertex( 10,  10, 60), 0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 2);

	// Ground
	Triangle T11(Vertex(-10, -10, 40),  Vertex( 10, -10, 40), Vertex(-10, -10, 60), 0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 1);
	Triangle T12(Vertex( 10, -10, 60),  Vertex(-10, -10, 60), Vertex( 10, -10, 40), 0.3, 0.7, 0.0, 0.0, 0.0, 0.0, Color::White, 2);

	Shape* Shapes[12] =
	{
		&T1, &T2, &T3, &T4, &T5, &T6, &T7, &T8, &T9, &T10, &T11, &T12
	};

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

		private:
			System::ComponentModel::Container ^components;

		public:
			

	public:


	public:

		Bitmap myBitmap_1 = Bitmap::FromFile("..\\Textures\\TS.jpg");

		#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->surfacePictureBox = (gcnew System::Windows::Forms::PictureBox());
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
			// RayTracer
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(823, 474);
			this->Controls->Add(this->surfacePictureBox);
			this->KeyPreview = true;
			this->Name = L"RayTracer";
			this->Text = L"Rotating Cube on X, Y, Z axes using  W/S, A/D, Q/E";
			this->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &RayTracer::RayTracer_KeyPress);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->surfacePictureBox))->EndInit();
			this->ResumeLayout(false);

		}
		#pragma endregion
		
		#pragma region ShadeDiffuse ShadeSpecular ShadingModel

		Color ShadeDiffuse(Shape* S, Vertex iPoint)
		{
			Vertex light = Vertex(0, 30, 0);

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
			Vertex light = Vertex(0, 30, 0);

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
			return      (r * Rd + normal * (w - k)).Normalize(); // t
		}

		#pragma endregion

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

					int x = (int)(u * 500);
					int y = (int)(v * 500);

					x = x > 499 ? 499 : x;        x = x < 0 ? 0 : x;
					y = y > 499 ? 499 : y;        y = y < 0 ? 0 : y;

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

					int x = (int)(u * 500);
					int y = (int)(v * 500);

					x = x > 499 ? 499 : x;        x = x < 0 ? 0 : x;
					y = y > 499 ? 499 : y;        y = y < 0 ? 0 : y;

					S->ShapeColor = myBitmap_1.GetPixel(x, y);
				}
				break;
			}
		}
		#pragma endregion


		void rotateCubeY_CCW(Vertex center)					// Key D is pressed
		{
			for (int i = 0; i < 12; i++)
			{
				Vertex P = Shapes[i]->V0;

				P = P - center;
				float tmpX = P.X;

				P.X = P.X * 0.966F - P.Z * 0.259F;
				P.Z = tmpX * 0.259F + P.Z * 0.966F;

				P = P + center;

				Shapes[i]->V0 = P;


				P = Shapes[i]->V1;

				P = P - center;
				tmpX = P.X;

				P.X = P.X * 0.966F - P.Z * 0.259F;
				P.Z = tmpX * 0.259F + P.Z * 0.966F;

				P = P + center;

				Shapes[i]->V1 = P;


				P = Shapes[i]->V2;

				P = P - center;
				tmpX = P.X;

				P.X = P.X * 0.966F - P.Z * 0.259F;
				P.Z = tmpX * 0.259F + P.Z * 0.966F;

				P = P + center;

				Shapes[i]->V2 = P;
			}
		}

		void rotateCubeY_CW(Vertex center)					// Key A is pressed
		{
			for (int i = 0; i < 12; i++)
			{
				Vertex P = Shapes[i]->V0;

				P = P - center;
				float tmpX = P.X;

				P.X = P.X * 0.966F + P.Z * 0.259F;
				P.Z = -tmpX * 0.259F + P.Z * 0.966F;

				P = P + center;

				Shapes[i]->V0 = P;


				P = Shapes[i]->V1;

				P = P - center;
				tmpX = P.X;

				P.X = P.X * 0.966F + P.Z * 0.259F;
				P.Z = -tmpX * 0.259F + P.Z * 0.966F;

				P = P + center;

				Shapes[i]->V1 = P;


				P = Shapes[i]->V2;

				P = P - center;
				tmpX = P.X;

				P.X = P.X * 0.966F + P.Z * 0.259F;
				P.Z = -tmpX * 0.259F + P.Z * 0.966F;

				P = P + center;

				Shapes[i]->V2 = P;
			}
		}

		void rotateCubeX_CCW(Vertex center)					// Key S is pressed
		{
			for (int i = 0; i < 12; i++)
			{
				Vertex P = Shapes[i]->V0;

				P = P - center;
				float tmpY = P.Y;

				P.Y = P.Y * 0.966F + P.Z * 0.259F;
				P.Z = -tmpY * 0.259F + P.Z * 0.966F;

				P = P + center;

				Shapes[i]->V0 = P;


				P = Shapes[i]->V1;

				P = P - center;
				tmpY = P.Y;

				P.Y = P.Y * 0.966F + P.Z * 0.259F;
				P.Z = -tmpY * 0.259F + P.Z * 0.966F;

				P = P + center;

				Shapes[i]->V1 = P;


				P = Shapes[i]->V2;

				P = P - center;
				tmpY = P.Y;

				P.Y = P.Y * 0.966F + P.Z * 0.259F;
				P.Z = -tmpY * 0.259F + P.Z * 0.966F;

				P = P + center;

				Shapes[i]->V2 = P;
			}
		}

		void rotateCubeX_CW(Vertex center)					// Key W is pressed
		{
			for (int i = 0; i < 12; i++)
			{
				Vertex P = Shapes[i]->V0;

				P = P - center;
				float tmpY = P.Y;

				P.Y = P.Y * 0.966F - P.Z * 0.259F;
				P.Z = tmpY * 0.259F + P.Z * 0.966F;

				P = P + center;

				Shapes[i]->V0 = P;


				P = Shapes[i]->V1;

				P = P - center;
				tmpY = P.Y;

				P.Y = P.Y * 0.966F - P.Z * 0.259F;
				P.Z = tmpY * 0.259F + P.Z * 0.966F;

				P = P + center;

				Shapes[i]->V1 = P;


				P = Shapes[i]->V2;

				P = P - center;
				tmpY = P.Y;

				P.Y = P.Y * 0.966F - P.Z * 0.259F;
				P.Z = tmpY * 0.259F + P.Z * 0.966F;

				P = P + center;

				Shapes[i]->V2 = P;
			}
		}

		void rotateCubeZ_CCW(Vertex center)					// Key Q is pressed
		{
			for (int i = 0; i < 12; i++)
			{
				Vertex P = Shapes[i]->V0;

				P = P - center;
				float tmpX = P.X;

				P.X = P.X * 0.966F - P.Y * 0.259F;
				P.Y = tmpX * 0.259F + P.Y * 0.966F;

				P = P + center;

				Shapes[i]->V0 = P;


				P = Shapes[i]->V1;

				P = P - center;
				tmpX = P.X;

				P.X = P.X * 0.966F - P.Y * 0.259F;
				P.Y = tmpX * 0.259F + P.Y * 0.966F;

				P = P + center;

				Shapes[i]->V1 = P;


				P = Shapes[i]->V2;

				P = P - center;
				tmpX = P.X;

				P.X = P.X * 0.966F - P.Y * 0.259F;
				P.Y = tmpX * 0.259F + P.Y * 0.966F;

				P = P + center;

				Shapes[i]->V2 = P;
			}
		}


		void rotateCubeZ_CW(Vertex center)					// Key E is pressed
		{
			for (int i = 0; i < 12; i++)
			{
				Vertex P = Shapes[i]->V0;

				P = P - center;
				float tmpX = P.X;

				P.X = P.X * 0.966F + P.Y * 0.259F;
				P.Y = -tmpX * 0.259F + P.Y * 0.966F;

				P = P + center;

				Shapes[i]->V0 = P;


				P = Shapes[i]->V1;

				P = P - center;
				tmpX = P.X;

				P.X = P.X * 0.966F + P.Y * 0.259F;
				P.Y = -tmpX * 0.259F + P.Y * 0.966F;

				P = P + center;

				Shapes[i]->V1 = P;


				P = Shapes[i]->V2;

				P = P - center;
				tmpX = P.X;

				P.X = P.X * 0.966F + P.Y * 0.259F;
				P.Y = -tmpX * 0.259F + P.Y * 0.966F;

				P = P + center;

				Shapes[i]->V2 = P;
			}
		}

		Color TraceRay(Vertex Ro, Vertex Rd, Shape* Shapes[], Vertex camera, int depth, Shape* prevShape)
		{
			if (depth > 2)
			{
				return prevShape->ShapeColor;
				//return Color::Black;
			}

			vector<intersections> Intersections;

			for (int i = 0; i < 12; i++)
			{
				float t = Shapes[i]->Intersect(Ro, Rd);
				if (t > 0.1F)
				{
					intersection.distance = t;
					intersection.indice = i;

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

		private: System::Void RayTracer_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e)
		{
			Bitmap^ surface = gcnew Bitmap(800, 450);
			surfacePictureBox->Image = surface;

			switch (e->KeyChar)
			{
				case 'a':
				{
					RayTracer::Text = "Key A is pressed : Clockwise rotation around Y-axis                                                                                    ( X : W/S, Y : A/D, Z : Q/E )";

					rotateCubeY_CW(Vertex(0, 0, 50));
				}
				break;

				case 'd':
				{
					RayTracer::Text = "Key D is pressed : Counterclockwise rotation around Y-axis                                                                      ( X : W/S, Y : A/D, Z : Q/E )";

					rotateCubeY_CCW(Vertex(0, 0, 50));
				}
				break;

				case 'w':
				{
					RayTracer::Text = "Key W is pressed : Clockwise rotation around X-axis                                                                                   ( X : W/S, Y : A/D, Z : Q/E )";

					rotateCubeX_CW(Vertex(0, 0, 50));

				}
				break;

				case 's':
				{
					RayTracer::Text = "Key S is pressed : Counterclockwise rotation around X-axis                                                                       ( X : W/S, Y : A/D, Z : Q/E )";

					rotateCubeX_CCW(Vertex(0, 0, 50));
				}
				break;

				case 'e':
				{
					RayTracer::Text = "Key E is pressed : Clockwise rotation around Z-axis                                                                                    ( X : W/S, Y : A/D, Z : Q/E )";

					rotateCubeZ_CW(Vertex(0, 0, 50));
				}
				break;

				case 'q':
				{
					RayTracer::Text = "Key Q is pressed : Counterclockwise rotation around Z-axis                                                                      ( X : W/S, Y : A/D, Z : Q/E )";

					rotateCubeZ_CCW(Vertex(0, 0, 50));

				}
				break;
				
			}

			for (int y = 0; y < 450; y++)
			{
				for (int x = 0; x < 800; x++)
				{                           //------u*(P1-P0)----     ------v*(P3-P0)----
					Vertex pixel	= P0 + ((float)x/800)*(P1-P0) + ((float)y/450)*(P3-P0);
					Vertex Rd		= (pixel - camera).Normalize();
					Color c			= TraceRay(camera, Rd, Shapes, camera, 0, NULL);
					surface->SetPixel(x, y, c);
				}
				if ((y % 20) == 0) surfacePictureBox->Refresh();
			}
			surfacePictureBox->Refresh();
		}
	};
}