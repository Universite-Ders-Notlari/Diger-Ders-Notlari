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

		private:
			System::ComponentModel::Container ^components;

		public:
			

	public:


	public:

		int rotLeft = 0;
	private: System::Windows::Forms::Button^  button1;
	public:

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
			this->button1 = (gcnew System::Windows::Forms::Button());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->surfacePictureBox))->BeginInit();
			this->SuspendLayout();
			// 
			// surfacePictureBox
			// 
			this->surfacePictureBox->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->surfacePictureBox->Location = System::Drawing::Point(16, 15);
			this->surfacePictureBox->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->surfacePictureBox->Name = L"surfacePictureBox";
			this->surfacePictureBox->Size = System::Drawing::Size(800, 450);
			this->surfacePictureBox->TabIndex = 0;
			this->surfacePictureBox->TabStop = false;
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(367, 484);
			this->button1->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(100, 28);
			this->button1->TabIndex = 1;
			this->button1->Text = L"Solution";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &RayTracer::button1_Click);
			// 
			// RayTracer
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(828, 522);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->surfacePictureBox);
			this->KeyPreview = true;
			this->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->Name = L"RayTracer";
			this->Text = L"Interactive Ray Tracing                                                          "
				L"                                                                           Use :"
				L" W, A, S, D, Z, X";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->surfacePictureBox))->EndInit();
			this->ResumeLayout(false);

		}
		#pragma endregion
		
		#pragma region ShadeDiffuse ShadeSpecular ShadingModel

		Color ShadeDiffuse(Shape* S, Vertex intersectionPoint)
		{
			Vertex light = Vertex(0, 80, 60);

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
			Vertex light = Vertex(0, 80, 60);

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

		private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) 
		{
			//
			// 2017 BAHAR VIZE 3. SORU SOLUTION
			//	

			Vertex N(0, 0.6, -0.8);
			Vertex camera(0, 0, 0);
			Vertex Rd(0, 0, 1);
			Vertex iPoint(0, 0, 100);

			Vertex P0(-8,  4.5, 10);
			Vertex P1( 8,  4.5, 10);
			Vertex P2( 8, -4.5, 10);
			Vertex P3(-8, -4.5, 10);

			P0 = P0 + 80 * Rd;
			P1 = P1 + 80 * Rd;
			P2 = P2 + 80 * Rd;
			P3 = P3 + 80 * Rd;
			
			float P_Length = 13.5739;

			//float P_Length = (iPoint - P0).Length();

			String^ message = "  camera P Distance = " + P_Length + "\n";
			MessageBox::Show(message);

			P0 = Vertex( 0.5894, -0.3315, 0.7367);
			P1 = Vertex(-0.5894, -0.3315, 0.7367);
			P2 = Vertex(-0.5894,  0.3315, 0.7367);
			P3 = Vertex( 0.5894,  0.3315, 0.7367);

			//P0 = (iPoint - P0).Normalize();
			//P1 = (iPoint - P1).Normalize();
			//P2 = (iPoint - P2).Normalize();
			//P3 = (iPoint - P3).Normalize();

			message =
				"(iPoint - P0).Normalize() = ( " + P0.X.ToString() + ", " + P0.Y.ToString() + ", " + P0.Z.ToString() + " ) \n" +
				"(iPoint - P1).Normalize() = ( " + P1.X.ToString() + ", " + P1.Y.ToString() + ", " + P1.Z.ToString() + " ) \n" +
				"(iPoint - P2).Normalize() = ( " + P2.X.ToString() + ", " + P2.Y.ToString() + ", " + P2.Z.ToString() + " ) \n" +
				"(iPoint - P3).Normalize() = ( " + P3.X.ToString() + ", " + P3.Y.ToString() + ", " + P3.Z.ToString() + " )";
			MessageBox::Show(message);

			P0 = Vertex( 0.5894,  0.6145, -0.5246);
			P1 = Vertex(-0.5894,  0.6145, -0.5246);
			P2 = Vertex(-0.5894,  0.8001,  0.1119);
			P3 = Vertex( 0.5894,  0.8001,  0.1119);

			//P0 = P0 - 2 * (P0*N) * N;
			//P1 = P1 - 2 * (P1*N) * N;
			//P2 = P2 - 2 * (P2*N) * N;
			//P3 = P3 - 2 * (P3*N) * N;

			message =
				"reflected_P0 = ( " + P0.X.ToString() + ", " + P0.Y.ToString() + ", " + P0.Z.ToString() + " ) \n" +
				"reflected_P1 = ( " + P1.X.ToString() + ", " + P1.Y.ToString() + ", " + P1.Z.ToString() + " ) \n" +
				"reflected_P2 = ( " + P2.X.ToString() + ", " + P2.Y.ToString() + ", " + P2.Z.ToString() + " ) \n" +
				"reflected_P3 = ( " + P3.X.ToString() + ", " + P3.Y.ToString() + ", " + P3.Z.ToString() + " )";
			MessageBox::Show(message);

			P0 = iPoint + P_Length * P0;
			P1 = iPoint + P_Length * P1;
			P2 = iPoint + P_Length * P2;
			P3 = iPoint + P_Length * P3;

			message =
				"new P0, P1, P2, P3 vectors : \n\n" +
				"P0 = ( " + P0.X.ToString() + ", " + P0.Y.ToString() + ", " + P0.Z.ToString() + " ) \n" +
				"P1 = ( " + P1.X.ToString() + ", " + P1.Y.ToString() + ", " + P1.Z.ToString() + " ) \n" +
				"P2 = ( " + P2.X.ToString() + ", " + P2.Y.ToString() + ", " + P2.Z.ToString() + " ) \n" +
				"P3 = ( " + P3.X.ToString() + ", " + P3.Y.ToString() + ", " + P3.Z.ToString() + " )";
			MessageBox::Show(message);
		}
	};
}