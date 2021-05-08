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
			this->Render->Location = System::Drawing::Point(376, 479);
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
			this->label1->Location = System::Drawing::Point(604, 482);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(136, 16);
			this->label1->TabIndex = 2;
			this->label1->Text = L"Rendering Time (sn) :";
			// 
			// timeBox
			// 
			this->timeBox->Font = (gcnew System::Drawing::Font(L"Consolas", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(162)));
			this->timeBox->Location = System::Drawing::Point(742, 477);
			this->timeBox->Name = L"timeBox";
			this->timeBox->Size = System::Drawing::Size(68, 26);
			this->timeBox->TabIndex = 3;
			this->timeBox->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// RayTracer
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(823, 514);
			this->Controls->Add(this->timeBox);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->Render);
			this->Controls->Add(this->surfacePictureBox);
			this->Name = L"RayTracer";
			this->Text = L"RayTracer : Sphere";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->surfacePictureBox))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
		#pragma endregion

		Color TraceRay(Vertex Ro, Vertex Rd, Triangle T, Sphere S)
		{
				float t_1 = S.Intersect(Ro, Rd);
				String^ message = " t_1 : " + t_1.ToString(); MessageBox::Show(message);

				Vertex iPoint_1 = Ro + t_1*Rd;
						message = " iPoint_1 (Sphere) = ( " +
									iPoint_1.X.ToString() + ", " +
									iPoint_1.Y.ToString() + ", " +
									iPoint_1.Z.ToString() + " )";
				MessageBox::Show(message);

				Vertex normal_1 = (S.Center - iPoint_1) / S.Radius;
						message = " normal_1 = ( " +
									normal_1.X.ToString() + ", " +
									normal_1.Y.ToString() + ", " +
									normal_1.Z.ToString() + " )";
				MessageBox::Show(message);

				Vertex Rref_1 = (Rd - 2 * (normal_1 * Rd) * normal_1).Normalize();
						message = " Rref_1 = ( " +
									Rref_1.X.ToString() + ", " +
									Rref_1.Y.ToString() + ", " +
									Rref_1.Z.ToString() + " )";
				MessageBox::Show(message);

				float t2 = S.Intersect(iPoint_1, Rref_1);
				message = " t_2 : " + t2.ToString(); MessageBox::Show(message);

				Vertex iPoint_2 = iPoint_1 + t2*Rref_1;
						message = " iPoint_2 (Sphere) = ( " +
									iPoint_2.X.ToString() + ", " +
									iPoint_2.Y.ToString() + ", " +
									iPoint_2.Z.ToString() + " )";
				MessageBox::Show(message);

				Vertex normal_2 = (S.Center - iPoint_2) / S.Radius;
						message = " normal_2 = ( " +
									normal_2.X.ToString() + ", " +
									normal_2.Y.ToString() + ", " +
									normal_2.Z.ToString() + " )";
				MessageBox::Show(message);

				Vertex Rref_2 = (Rref_1 - 2 * (normal_2 * Rref_1) * normal_2).Normalize();
						message = " Rref_2 = ( " +
									Rref_2.X.ToString() + ", " +
									Rref_2.Y.ToString() + ", " +
									Rref_2.Z.ToString() + " )";
				MessageBox::Show(message);

				float t3 = T.Intersect(iPoint_2, Rref_2);
				message = " t_3 : " + t3.ToString(); MessageBox::Show(message);

				Vertex iPoint_3 = iPoint_2 + t3*Rref_2;
						message = " iPoint_3 (Triangle) = ( " +
									iPoint_3.X.ToString() + ", " +
									iPoint_3.Y.ToString() + ", " +
									iPoint_3.Z.ToString() + " )";
				MessageBox::Show(message);

				return S.ShapeColor;
		}

		private: System::Void Render_Click(System::Object^  sender, System::EventArgs^  e) 
		{
			DateTime startTime; 
			startTime = startTime.Now;

			Bitmap^ surface = gcnew Bitmap(800, 450);
			surfacePictureBox->Image = surface;

			Triangle T(Vertex(-25, 15, 0), Vertex(-15, 15, 15), Vertex(-15, 15, -15), Color::Red);
			Sphere   S(Vertex(0, 0, 0), 40, Color::Blue);

			Vertex camera	= Vertex(24, -12, 0);
			Vertex Rd		= Vertex(0.8, 0.6, 0);
			Color c			= TraceRay(camera, Rd, T, S);

			DateTime endTime;
			endTime = endTime.Now;
			TimeSpan deltaTime = endTime - startTime;
			timeBox->Text = (Math::Round(100 * deltaTime.TotalSeconds) / 100.0F).ToString();

			DateTime date;
			String^ filename = date.Now.Year.ToString() + date.Now.Month.ToString() + date.Now.Day.ToString() +
				date.Now.Hour.ToString() + date.Now.Minute.ToString() + date.Now.Second.ToString() + ".jpg";
			surface->Save("image/" + filename);
		}
	};
}