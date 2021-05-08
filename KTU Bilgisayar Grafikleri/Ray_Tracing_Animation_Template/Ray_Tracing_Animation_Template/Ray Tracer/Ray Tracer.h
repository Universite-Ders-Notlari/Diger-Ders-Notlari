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

	int t = 0;

	Vertex camera( 0, 0,  0 );
	Vertex P0( -8,  4.5, 10 );
	Vertex P1(  8,  4.5, 10 );
	Vertex P2(  8, -4.5, 10 );
	Vertex P3( -8, -4.5, 10 );

	// Red Sphere
	Sphere S1( Vertex( 0, -30, 140 ), 50, 0.3F, 0.7F, 0, 0, 0, 0, Color::Red, 0);

	// iPoint Red
	Sphere S2( Vertex( 0,   0, 100 ), 0.5, 0.3F, 0.7F, 0, 0, 0, 0, Color::White, 0);

	// Blue Sphere
	Sphere S3( Vertex( 0, 144,  68 ), 50, 0.3F, 0.7F, 0, 0, 0, 0, Color::Blue, 0);

	// iPoint Blue
	Sphere S4( Vertex( 0,  96,  72 ), 2, 0.3F, 0.7F, 0, 0, 0, 0, Color::White, 0);

	Shape* Shapes[4] =
	{
		&S1, &S2, &S3, &S4
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

		int rotLeft = 0;

	public:

	public:
		int rotRight = 0;

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
			this->Text = L"RAY TRACING ANIMATION UYGULAMA DERSI                                             "
				L"                                                         Use : W, A, S, D, Z, X";
			this->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &RayTracer::RayTracer_KeyPress);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->surfacePictureBox))->EndInit();
			this->ResumeLayout(false);

		}
		#pragma endregion
		
		#pragma region ShadeDiffuse ShadeSpecular ShadingModel

		Color ShadeDiffuse(Shape* S, Vertex intersectionPoint)
		{
			Vertex light = Vertex(0, 30, 60);

			Vertex toLight = (light - intersectionPoint).Normalize();
			Vertex normal = S->NormalAt(intersectionPoint).Normalize();
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

		Color ShadeSpecular(Shape* S, Vertex intersectionPoint, Vertex camera)
		{
			Vertex light = Vertex(0, 30, 60);

			Vertex fromLight = (intersectionPoint - light).Normalize();
			Vertex normal = S->NormalAt(intersectionPoint).Normalize();
			Vertex toCamera = (camera - intersectionPoint).Normalize();

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

		Vertex CalculateReflection(Shape* S, Vertex intersectionPoint, Vertex Rd)
		{
			Vertex normal = S->NormalAt(intersectionPoint).Normalize();
			return (Rd - 2 * (normal * Rd) * normal).Normalize();
		}

		Vertex CalculateTransmission(Shape* S, Vertex intersectionPoint, Vertex Rd)
		{
			return Rd;
		}

		Vertex CalculateRefraction(Shape* S, Vertex intersectionPoint, Vertex Rd, float n1, float n2)
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

			Vertex normal = S->NormalAt(intersectionPoint).Normalize();
			float       r = n1 / n2;
			float       w = -(Rd * normal) * r;
			float       k = (float)Math::Sqrt((double)(1 + (w - r) * (w + r)));
			return      (r * Rd + normal * (w - k)).Normalize(); // t
		}

		#pragma endregion

		#pragma region Shadow Test

		bool TestShadow(Shape* Shapes[], Vertex intersectionPoint)
		{
			Vertex light = Vertex(0, 30, 60);

			Vertex toLight = (light - intersectionPoint).Normalize();

			vector<intersections> Intersections;

			if (AABB(intersectionPoint, toLight, Vertex(-25, -20, 70), 15, 20, 30) > 0.0)
			{
				for (int i = 18; i < 96; i++)
				{
					float t = Shapes[i]->Intersect(intersectionPoint, toLight);
					if (t > 0.1F)							// 0.1 for avoiding self intersection at reflection
					{
						intersection.distance = t;
						intersection.indice   = i;

						Intersections.push_back(intersection);
					}
				}
			}

			for (int i = 0; i < 18; i++)
			{
				float t = Shapes[i]->Intersect(intersectionPoint, toLight);
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

				if (min_distance < (light - intersectionPoint).Length())
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
			float tmax = 1000;
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

		#pragma region rotateLeft
		// Counterclockwise rotation around y-axis
		// http://www.mathworks.com/help/phased/ref/roty.html
		Vertex rotateLeft(Vertex P, Vertex camera)
		{
			P = P - camera;
			float tmpX = P.X;

			P.X =  P.X * 0.966F - P.Z * 0.259F;
			P.Z = tmpX * 0.259F + P.Z * 0.966F;

			P = P + camera;
			return P;
		}
		#pragma endregion

		#pragma region rotateRight
		// Clockwise rotation around y-axis
		Vertex rotateRight(Vertex P, Vertex camera)
		{
			P = P - camera;
			float tmpX = P.X;

			P.X =   P.X * 0.966F + P.Z * 0.259F;
			P.Z = -tmpX * 0.259F + P.Z * 0.966F;

			P = P + camera;
			return P;
		}
		#pragma endregion

		Color TraceRay(Vertex Ro, Vertex Rd, Shape* Shapes[], Vertex camera, int depth, Shape* prevShape)
		{
			if (depth > 2)
			{
				return prevShape->ShapeColor;
				//return Color::Black;
			}

			vector<intersections> Intersections;

			for (int i = 0; i < 4; i++)
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

				Vertex intersectionPoint = Ro + min_distance * Rd;			
				Shape* S                 = Shapes[min_indis];

				Color reflectedColor;
				if (S->Refl != 0.0F)
				{
					Vertex reflectedDirection = CalculateReflection(S, intersectionPoint, Rd);
					reflectedColor = TraceRay(intersectionPoint, reflectedDirection, Shapes, camera, depth + 1, S);
				}

				Color transmittedColor;
				if (S->Trans != 0.0F)
				{
					Vertex transmittedDirection = CalculateTransmission(S, intersectionPoint, Rd);
					transmittedColor = TraceRay(intersectionPoint, transmittedDirection, Shapes, camera, depth + 1, S);
				}

				Color refractedColor;
				if (S->Refrac != 0.0F)
				{
					Vertex refractedDirection = CalculateRefraction(S, intersectionPoint, Rd, 1.0F, 1.33F);
					refractedColor = TraceRay(intersectionPoint, refractedDirection, Shapes, camera, depth + 1, S);
				}

				Color diffuseColor  = ShadeDiffuse(S, intersectionPoint);
				Color specularColor = ShadeSpecular(S, intersectionPoint, camera);

				return ShadingModel(S, diffuseColor, specularColor, reflectedColor, transmittedColor, refractedColor, S->Ambient, S->Dif, S->Spec, S->Refl, S->Trans, S->Refrac);
			}

			return Color::Black;
		}

		private: System::Void RayTracer_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e)
		{
			Bitmap^ surface = gcnew Bitmap(800, 450);
			surfacePictureBox->Image = surface;

			// RAY TRACING ANIMATION UYGULAMA DERSI ADIMLARI :
			// 1. ASAGIDAKI KODU BIR DONGU ICINE ALIP KIRMIZI KÜREYE KADAR GIDECEKSINIZ.
			// 2. P0, P1, P2 VE P3'ÜN YANSIMALARINI HESAPLAYACAKSINIZ. 
			// 3. YINE DONGU KURUP BU SEFER MAVI KUREYE GIDECEKSINIZ.

			Vertex norm = (P1 - P0).CrossProduct(P2 - P1);
			norm.Normalize();

			P0 = P0 - 5 * norm;
			P1 = P1 - 5 * norm;
			P2 = P2 - 5 * norm;
			P3 = P3 - 5 * norm;

			camera = (P0 + P2) / 2 + 10 * norm;

			for (int y = 0; y < 450; y++)
			{
				for (int x = 0; x < 800; x++)
				{                         //---u*(P1-P0)---     ---v*(P3-P0)---
					Vertex pixel = P0 + (x / 799.0)*(P1 - P0) + (y / 449.0)*(P3 - P0);
					Vertex Rd = (pixel - camera).Normalize();
					Color c = TraceRay(camera, Rd, Shapes, camera, 0, NULL);
					surface->SetPixel(x, y, c);
				}
				if ((y % 20) == 0) surfacePictureBox->Refresh();
			}

			t++;
			String^ filename = t.ToString() + ".jpg";
			surface->Save("image/" + filename);
		}
	};
}