//--------------------------------------------------------------------------------------
//
//                  2014-2015 Bahar Dönemi BIL320 Bilgisayar Grafikleri I
//                  Dönem Projesi : "DirectX 11 ile 3D Satranç Oyunu"
//
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <mmsystem.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "resource.h"
#include <DXGI.h>
#include <dinput.h>
#include <fstream>
#include <sstream>

using namespace std;

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
	XMFLOAT3 Normal;
};

struct ConstantBuffer
{  
	XMMATRIX mWorld;
	XMMATRIX mworldInvTranspose;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 vEyePos;
	XMFLOAT4 vLightPos[2];
	XMFLOAT4 vMeshColor;
	XMFLOAT4 vLightColor[2];
}cBuffer;


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE                           g_hInst = NULL;
HWND                                g_hWnd = NULL;
D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL                   g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*                       g_pd3dDevice = NULL;
ID3D11DeviceContext*                g_pImmediateContext = NULL;
IDXGISwapChain*                     g_pSwapChain = NULL;
ID3D11RenderTargetView*             g_pRenderTargetView = NULL;
ID3D11Texture2D*                    g_pDepthStencil = NULL;
ID3D11DepthStencilView*             g_pDepthStencilView = NULL;
ID3D11VertexShader*                 g_pVertexShader = NULL;
ID3D11PixelShader*                  g_pPixelShader = NULL;
ID3D11PixelShader*                  g_pPixelShader_Textured = NULL;
ID3D11PixelShader*                  g_pPixelShader_Solid = NULL;
ID3D11InputLayout*                  g_pVertexLayout = NULL;
ID3D11Buffer*                       g_pVertexBuffer_Pointer = NULL;
ID3D11Buffer*                       g_pVertexBuffer_CrossHair = NULL;
ID3D11Buffer*                       g_pVertexBuffer_chessBoard = NULL;
ID3D11Buffer*                       g_pVertexBuffer_Light = NULL;
ID3D11Buffer*                       g_pIndexBuffer = NULL;
ID3D11Buffer*                       g_pConstantBuffer = NULL;

//Benim kullandýgým doku kaplamalarý icin
//---------------------------------------------------------------------
ID3D11ShaderResourceView*           g_pTexturepointerWhite = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerBlack = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerWhite_2 = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerBlack_2 = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerWhite_3 = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerBlack_3 = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerWhite_4 = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerBlack_4 = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerWhite_5 = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerBlack_5 = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerWhite_6 = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerBlack_6 = NULL;

ID3D11ShaderResourceView*           g_pTexturepointerWhiteRed = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerBlackRed = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerWhiteRed_2 = NULL;
ID3D11ShaderResourceView*           g_pTexturepointerBlackRed_2 = NULL;

ID3D11ShaderResourceView*           g_pTexturechessBoard_2 = NULL;
//---------------------------------------------------------------------

ID3D11ShaderResourceView*           g_pTexturechessBoard = NULL;
ID3D11SamplerState*                 g_pSamplerLinear = NULL;
XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;

//Taslar icin kullandýgým VertexBuffer'lar
//---------------------------------------------------------------------
ID3D11Buffer*                       g_pVertexBuffer_Piyon = NULL;
ID3D11Buffer*						g_pVertexBuffer_At = NULL;
ID3D11Buffer*						g_pVertexBuffer_Fil = NULL;
ID3D11Buffer*						g_pVertexBuffer_Kale = NULL;
ID3D11Buffer*						g_pVertexBuffer_Vezir = NULL;
ID3D11Buffer*						g_pVertexBuffer_Sah = NULL;

ID3D11Buffer*                       g_pVertexBuffer_PointerHamle = {NULL};
//---------------------------------------------------------------------

//Secilen Pointerlarýn turuldugu matris 1 2 ve 3 degerleri alýr sarý kýrmýzý ve secýlen halka seklýnde gosterým ýcýn 
int renderPointerHamle[8][8] = {{0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0}};
//-------------------------------------------------------------------

//Taslar icin VertexCount'lar
//-------------------------------------------------------------------
int vertexCountPiyon;
int vertexCountAt;
int vertexCountFil;
int vertexCountKale;
int vertexCountVezir;
int vertexCountSah;
//-------------------------------------------------------------------

//Satranç tahtasýnýn bolmelerýnýn koordintlarýný tutan matrisler. Burada 8 den sonrasý yenen taslarýn tutulacagý koordýnatlardýr
//-------------------------------------------------------------------------------------------------
float satrancTahtasiX[16] = {-17.5,-12.5,-7.5,-2.5,2.5,7.5,12.5,17.5,-17.5,-12.5,-7.5,-2.5,2.5,7.5,12.5,17.5};
float satrancTahtasiZ[12] = {-17.5,-12.5,-7.5,-2.5,2.5,7.5,12.5,17.5,27.5,-27.5,32.5,-32.5};

//Burada oyun icin kullanýlan degiþkenler var
//----------------------------------------------
bool oyunSirasi = true;
float tasHareketHiz = 1.5; //Bu deger Render() fonksiyonu icerisinde render hýzýna baglý olarak ayarlanýyor.
float tasYmax = 7.0;
bool hamleDurum = false;
bool renderPointerHamleDurum = true;
bool sahTehlikeDurum[2] = {false,false};
bool sahTehlikeDurumRender[2] = {false,false};
int piyon2Hamle[2] = {0,0};
//----------------------------------------------

//Taslarýn hareket halýnde olup olmadýklarýnýn kontrolu ýcýn kullanýlan matris.
bool hareketVar[12] = {false,false,false,false,false,false,false,false,false,false,false,false};

bool piyonBeyazY[8] = {true,true,true,true,true,true,true,true};
bool piyonSiyahY[8] = {true,true,true,true,true,true,true,true};
bool atBeyazY[2] = {true,true};
bool atSiyahY[2] = {true,true};
bool filBeyazY[2] = {true,true};
bool filSiyahY[2] = {true,true};
bool kaleBeyazY[2] = {true,true};
bool kaleSiyahY[2] = {true,true};
bool vezirBeyazY = true;
bool vezirSiyahY = true;
bool sahBeyazY = true;
bool sahSiyahY = true;

//Taslar icin Renk vektorleri (R,G,B)
XMFLOAT4 WhiteColor = XMFLOAT4(1.0f,1.0f,1.0f,1.0f);
XMFLOAT4 BlackColor = XMFLOAT4(0.2f,0.2f,0.2f,1.0f);
XMFLOAT4 RedColor = XMFLOAT4(1.0f,0.0f,0.0f,1.0f);
//--------------------------------------

//Oyun durum matrisi
int oyunDurum[8][8][2] = {{{4,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{4,2}},
						 {{2,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{2,2}},
						 {{3,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{3,2}},
					     {{5,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{5,2}},
						 {{6,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{6,2}},
						 {{3,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{3,2}},
						 {{2,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{2,2}},
						 {{4,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{4,2}}};

//burada taslarýn tutuldugu koordinatlar olusturuldu baslangýc ýcýn
float piyonBeyazKonum[8][3] = {{satrancTahtasiX[0],0,satrancTahtasiZ[1]},{satrancTahtasiX[1],0,satrancTahtasiZ[1]},{satrancTahtasiX[2],0,satrancTahtasiZ[1]},{satrancTahtasiX[3],0,satrancTahtasiZ[1]},{satrancTahtasiX[4],0,satrancTahtasiZ[1]},{satrancTahtasiX[5],0,satrancTahtasiZ[1]},{satrancTahtasiX[6],0,satrancTahtasiZ[1]},{satrancTahtasiX[7],0,satrancTahtasiZ[1]}};
float piyonSiyahKonum[8][3] = {{satrancTahtasiX[0],0,satrancTahtasiZ[6]},{satrancTahtasiX[1],0,satrancTahtasiZ[6]},{satrancTahtasiX[2],0,satrancTahtasiZ[6]},{satrancTahtasiX[3],0,satrancTahtasiZ[6]},{satrancTahtasiX[4],0,satrancTahtasiZ[6]},{satrancTahtasiX[5],0,satrancTahtasiZ[6]},{satrancTahtasiX[6],0,satrancTahtasiZ[6]},{satrancTahtasiX[7],0,satrancTahtasiZ[6]}};
float atBeyazKonum[2][3] = {{satrancTahtasiX[1],0,satrancTahtasiZ[0]},{satrancTahtasiX[6],0,satrancTahtasiZ[0]}};
float atSiyahKonum[2][3] = {{satrancTahtasiX[1],0,satrancTahtasiZ[7]},{satrancTahtasiX[6],0,satrancTahtasiZ[7]}};
float filBeyazKonum[2][3] = {{satrancTahtasiX[2],0,satrancTahtasiZ[0]},{satrancTahtasiX[5],0,satrancTahtasiZ[0]}};
float filSiyahKonum[2][3] = {{satrancTahtasiX[2],0,satrancTahtasiZ[7]},{satrancTahtasiX[5],0,satrancTahtasiZ[7]}};
float kaleBeyazKonum[2][3] = {{satrancTahtasiX[0],0,satrancTahtasiZ[0]},{satrancTahtasiX[7],0,satrancTahtasiZ[0]}};
float kaleSiyahKonum[2][3] = {{satrancTahtasiX[0],0,satrancTahtasiZ[7]},{satrancTahtasiX[7],0,satrancTahtasiZ[7]}};
float vezirBeyazKonum[3] = {satrancTahtasiX[3],0,satrancTahtasiZ[0]};
float vezirSiyahKonum[3] = {satrancTahtasiX[3],0,satrancTahtasiZ[7]};
float sahBeyazKonum[3] = {satrancTahtasiX[4],0,satrancTahtasiZ[0]};
float sahSiyahKonum[3] = {satrancTahtasiX[4],0,satrancTahtasiZ[7]};
//-----------------------------------------------------------------
//burada taslarýn tutuldugu koordinatlar olusturuldu baslangýc ýcýn
float piyonBeyazKonumHareket[8][3] = {{satrancTahtasiX[0],0,satrancTahtasiZ[1]},{satrancTahtasiX[1],0,satrancTahtasiZ[1]},{satrancTahtasiX[2],0,satrancTahtasiZ[1]},{satrancTahtasiX[3],0,satrancTahtasiZ[1]},{satrancTahtasiX[4],0,satrancTahtasiZ[1]},{satrancTahtasiX[5],0,satrancTahtasiZ[1]},{satrancTahtasiX[6],0,satrancTahtasiZ[1]},{satrancTahtasiX[7],0,satrancTahtasiZ[1]}};
float piyonSiyahKonumHareket[8][3] = {{satrancTahtasiX[0],0,satrancTahtasiZ[6]},{satrancTahtasiX[1],0,satrancTahtasiZ[6]},{satrancTahtasiX[2],0,satrancTahtasiZ[6]},{satrancTahtasiX[3],0,satrancTahtasiZ[6]},{satrancTahtasiX[4],0,satrancTahtasiZ[6]},{satrancTahtasiX[5],0,satrancTahtasiZ[6]},{satrancTahtasiX[6],0,satrancTahtasiZ[6]},{satrancTahtasiX[7],0,satrancTahtasiZ[6]}};
float atBeyazKonumHareket[2][3] = {{satrancTahtasiX[1],0,satrancTahtasiZ[0]},{satrancTahtasiX[6],0,satrancTahtasiZ[0]}};
float atSiyahKonumHareket[2][3] = {{satrancTahtasiX[1],0,satrancTahtasiZ[7]},{satrancTahtasiX[6],0,satrancTahtasiZ[7]}};
float filBeyazKonumHareket[2][3] = {{satrancTahtasiX[2],0,satrancTahtasiZ[0]},{satrancTahtasiX[5],0,satrancTahtasiZ[0]}};
float filSiyahKonumHareket[2][3] = {{satrancTahtasiX[2],0,satrancTahtasiZ[7]},{satrancTahtasiX[5],0,satrancTahtasiZ[7]}};
float kaleBeyazKonumHareket[2][3] = {{satrancTahtasiX[0],0,satrancTahtasiZ[0]},{satrancTahtasiX[7],0,satrancTahtasiZ[0]}};
float kaleSiyahKonumHareket[2][3] = {{satrancTahtasiX[0],0,satrancTahtasiZ[7]},{satrancTahtasiX[7],0,satrancTahtasiZ[7]}};
float vezirBeyazKonumHareket[3] = {satrancTahtasiX[3],0,satrancTahtasiZ[0]};
float vezirSiyahKonumHareket[3] = {satrancTahtasiX[3],0,satrancTahtasiZ[7]};
float sahBeyazKonumHareket[3] = {satrancTahtasiX[4],0,satrancTahtasiZ[0]};
float sahSiyahKonumHareket[3] = {satrancTahtasiX[4],0,satrancTahtasiZ[7]};
//-----------------------------------------------------------------

//Yenen taslar icin indis degerleri
int yenenBeyazX = 8;
int yenenSiyahX = 8;
int yenenBeyazZ = 8;
int yenenSiyahZ = 9;
//---------------------------------

int vertexCountpointer;
int vertexCountchessBoard;
int vertexCountlight;
int vertexCountcrosshair;

SimpleVertex* verticespointer;
SimpleVertex* verticeschessBoard;
SimpleVertex* verticesLight;
SimpleVertex* verticesCrossHair;

//benim eklediklerim
//------------------------------------------------------
SimpleVertex* verticesPiyon;
SimpleVertex* verticesAt;
SimpleVertex* verticesFil;
SimpleVertex* verticesKale;
SimpleVertex* verticesVezir;
SimpleVertex* verticesSah;

SimpleVertex* verticesPointerHamle;
//--------------------------------------------------------

XMVECTOR Ro;
XMVECTOR Rd ;


//-------------------------------------------------------------------------------------
// DIRECT INPUT
//-------------------------------------------------------------------------------------
IDirectInputDevice8* DIKeyboard;
IDirectInputDevice8* DIMouse;

DIMOUSESTATE mouseLastState;
LPDIRECTINPUT8 DirectInput;

float rot				= 0.01f;
double countsPerSecond	= 0.0;
__int64 CounterStart	= 0;
int frameCount			= 0;
int fps					= 0;
__int64 frameTimeOld	= 0;
double frameTime;

bool InitDirectInput(HINSTANCE hInstance);
void DetectInput(double time);
void UpdateScene(double time);
void StartTimer();
double GetTime();
double GetFrameTime();

//Oyun icin kullandýgým fonksiyonlarýn prototipleri
void SecilenTas(int *tas, bool *renk,int x,int z,bool *durum);
void HamleGoster(int tas, bool renk, int x, int z, bool sahKontrol);
void RenderPointerHamleReset();
bool RenderHamleBosKontrol(void);
void HamleYap(int secimTas, bool secimRenk, int secimX, int secimZ, int yeniX, int yeniZ);
void YenenTas(int X , int Z , bool renk);
void TasHareket(void);
bool KontrolTasHareket(void);
void SahTeklikeKontrol(void);
void Sesler(int ses);
void HamleKontol(int secimX, int secimZ,bool sahMat);
void SahMat(void);
void Pat(void);
void OyunYenile(void);
void HamleOlanTaslariGoster(void);
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// FIRST PERSON CAMERA
//-------------------------------------------------------------------------------------

XMVECTOR Eye ;
XMVECTOR At ;
XMVECTOR Up ;

XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR DefaultRight	= XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR camForward		= XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR camRight		= XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

XMVECTOR crosshairPosition= XMVectorSet(0.0f,0.0f,0.0f, 0.0f);

XMMATRIX camRotationMatrix;
XMMATRIX groundWorld;

float moveLeftRight = 0.0f;
float moveBackForward = 0.0f;

float camYaw = 0.0f;
float camPitch = 0.5f;

void UpdateCamera();

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render(double time);

XMVECTOR testIntersection(XMVECTOR rayOrigin, XMVECTOR rayDir, SimpleVertex* verticesModel, int vertexCount, bool transform, XMMATRIX& g_World);

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

	if(!InitDirectInput(hInstance))
	{
		MessageBox(0, L"Direct Input Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

	//Ýlk olarak Beyaz taslar için hamlesi olan taslarý gosterir
	HamleOlanTaslariGoster();
	//-------------------------

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
			frameCount++;
			if(GetTime() > 1.0f)
			{
				fps = frameCount;
				frameCount = 0;
				StartTimer();
			}	

			frameTime = GetFrameTime();

			DetectInput(frameTime);

            Render(frameTime);
        }
    }

    CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 600, 450 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"TutorialWindowClass", L"3D Satranç", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWnd )
        return E_FAIL;

	//Buraka pencere maksimize yapýldý
	ShowWindow( g_hWnd, SW_MAXIMIZE);
	//ShowWindow( g_hWnd, nCmdShow );
	//----------------------------------------------------------------
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}

bool InitDirectInput(HINSTANCE hInstance)
{
	HRESULT HR = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DirectInput, NULL); 
	HR = DirectInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL);
	HR = DirectInput->CreateDevice(GUID_SysMouse, &DIMouse, NULL);
	HR = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	HR = DIKeyboard->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	HR = DIMouse->SetDataFormat(&c_dfDIMouse);
	HR = DIMouse->SetCooperativeLevel(g_hWnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
	return true;
}

bool secim = false;
int secimX = 0;
int secimZ = 0;
int secimTas;
bool secimRenk;

void DetectInput(double time)
{
	DIMOUSESTATE mouseCurrState;
	BYTE keyboardState[256];
	DIKeyboard->Acquire();
	DIMouse->Acquire();

	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
	DIKeyboard->GetDeviceState(sizeof(keyboardState),(LPVOID)&keyboardState);
	if(keyboardState[DIK_ESCAPE] & 0x80) PostMessage(g_hWnd, WM_DESTROY, 0, 0);

	float speed = 15.0F * time;

	if(keyboardState[DIK_A] & 0x80)
	{
		moveLeftRight -= speed;
	}

	if(keyboardState[DIK_D] & 0x80)
	{
		moveLeftRight += speed;
	}

	if(keyboardState[DIK_W] & 0x80)
	{
		moveBackForward += speed;
	}

	if(keyboardState[DIK_S] & 0x80)
	{
		moveBackForward -= speed;
	}

	//Yeni Oyun Baþlat
	if(keyboardState[DIK_Y] & 0x80)
	{
		if(MessageBox(NULL, TEXT("Yeni oyun baþlatýlsýnmý ?"),TEXT("YENÝ OYUN"), MB_YESNO | MB_ICONINFORMATION) == IDYES){
			OyunYenile();
		}
	}
	//----------------------------

	//Mouse sol tus týklandýgýnda
	if(mouseCurrState.rgbButtons[0] & 0x80)
	{
		int tas;
		bool renk,durum = true;
		int X,Z;

		//Burada farenýn hangi tasýn hucresýne týkladýgýný buluyoruz
		//------------------------------------------------------------------------------------------------
		cBuffer.vMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		XMVECTOR translatePointer = testIntersection(Ro, Rd, verticeschessBoard, vertexCountchessBoard, false, cBuffer.mWorld);
		XMFLOAT3 translateP;
		XMStoreFloat3(&translateP, translatePointer);

		X = (translateP.x + 20) / 5.0f;
		Z = (translateP.z + 20) / 5.0f;
		//-----------------------------------------------------------------------------------------------

		int indis;
		if(oyunSirasi){
			indis = 1;
		}
		else{
			indis = 0;
		}

		//Burada hareket halýnde býr tas yoksa(dýger oyuncunun hamlesý sona ermisse) oyun sýrasý kýmde ise onun tas secme ve hamle yapma islemleri yapýlýr.
		if(KontrolTasHareket()){
			if(!secim || ((X != secimX || Z != secimZ) && (renderPointerHamle[X][Z] == 0 || renderPointerHamle[X][Z] == 5))){ //secilen tas oyuncununmu oyuncunun ýse ve hamle degilse sec
				RenderPointerHamleReset();
				SecilenTas(&tas,&renk,X,Z,&durum);
				if(durum & (oyunSirasi == renk)){
					//Secilen tas için ses
					PlaySound(TEXT("secilenTas.wav"), NULL, SND_FILENAME | SND_ASYNC);
					////////////////////////////

					HamleGoster(tas,renk,X,Z,false);
					//Gecerli olmayan hamle varmý yokmu kontrolü yap
					HamleKontol(X,Z,false);
					secim = true;
				}
				secimX = X;
				secimZ = Z;
				secimTas = tas;
				secimRenk = renk;
			}
			else{ //Secilen konum hamle yapýlacak konumsa hamleyý yap
				if(renderPointerHamle[X][Z] == 1 || renderPointerHamle[X][Z] == 2){
					HamleYap(secimTas,secimRenk,secimX,secimZ,X,Z);
					hamleDurum = true;
					secim = false;
					sahTehlikeDurumRender[0] = false;
					sahTehlikeDurumRender[1] = false;

					//Burada sah tehlike durumuna bakýlacak
					renderPointerHamleDurum = false;
					SahTeklikeKontrol();
					renderPointerHamleDurum = true;
					//------------------------------------

					if(sahTehlikeDurum[indis]){ //Sah tehlike durumunda ise sahTehlike sesi oynatýlacak
						sahTehlikeDurumRender[indis] = true;
						PlaySound(TEXT("sahTehlike.wav"), NULL, SND_FILENAME | SND_ASYNC);
					}
				}
			}
		}
	}
	//Mouse sað tus týklandýgýnda
	if(mouseCurrState.rgbButtons[1] & 0x80)
	{
		if(KontrolTasHareket()){
			//Hamle olan taslarý goster
			HamleOlanTaslariGoster();
			secim = false;	//secilmiþ tas varsa secýlmemýs hale getýr.
			//-------------------------
		}
	}

	if((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY))
	{
		camYaw += mouseLastState.lX * 0.001f;

		camPitch += mouseCurrState.lY * 0.001f;

		mouseLastState = mouseCurrState;
	}

	UpdateCamera();

	return;
}

void StartTimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);
	countsPerSecond = double(frequencyCount.QuadPart);
	QueryPerformanceCounter(&frequencyCount);
	CounterStart = frequencyCount.QuadPart;
}

double GetTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	return double(currentTime.QuadPart-CounterStart)/countsPerSecond;
}

double GetFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);

	tickCount = currentTime.QuadPart-frameTimeOld;
	frameTimeOld = currentTime.QuadPart;

	if(tickCount < 0.0f)
		tickCount = 0.0f;

	return float(tickCount)/countsPerSecond;
}

void UpdateScene(double time)
{
}

void UpdateCamera()
{
	camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);
	At = XMVector3TransformCoord(DefaultForward, camRotationMatrix );
	At = XMVector3Normalize(At);

	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = XMMatrixRotationY(camYaw);

	camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
	Up = XMVector3TransformCoord(Up, RotateYTempMatrix);
	camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);

	Eye += moveLeftRight * camRight;
	Eye += moveBackForward * camForward;

	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;

	At = Eye + At;
	crosshairPosition = Eye + 0.5 * XMVector3Normalize(At - Eye); 

	g_View = XMMatrixLookAtLH( Eye, At, Up );

}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
                                            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil );
    if( FAILED( hr ) )
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED( hr ) )
        return hr;

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    // Compile the vertex shader
    ID3DBlob* pVSBlob = NULL;
    hr = CompileShaderFromFile( L"D3D11_Chess.fx", "VS", "vs_4_0", &pVSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader );
    if( FAILED( hr ) )
    {    
        pVSBlob->Release();
        return hr;
    }

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },

	};
    UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
    pVSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

    // Compile the pixel shader
    ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile( L"D3D11_Chess.fx", "PS", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader );
    pPSBlob->Release();
    if( FAILED( hr ) )	return hr;

    // Compile the pixel shader Textured
    ID3DBlob* pPStextureBlob = NULL;
    hr = CompileShaderFromFile( L"D3D11_Chess.fx", "PS_Textured", "ps_4_0", &pPStextureBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader( pPStextureBlob->GetBufferPointer(), pPStextureBlob->GetBufferSize(), NULL, &g_pPixelShader_Textured );
    pPStextureBlob->Release();
    if( FAILED( hr ) )	return hr;


    // Compile the pixel shader Solid
    ID3DBlob* pPSsolidBlob = NULL;
    hr = CompileShaderFromFile( L"D3D11_Chess.fx", "PS_Solid", "ps_4_0", &pPSsolidBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader( pPSsolidBlob->GetBufferPointer(), pPSsolidBlob->GetBufferSize(), NULL, &g_pPixelShader_Solid );
    pPSsolidBlob->Release();
    if( FAILED( hr ) )	return hr;

	ifstream fin;

	// SATRANÇ TAHTASINI YÜKLE
	fin.open("chessBoard.txt");
	fin >> vertexCountchessBoard;
	verticeschessBoard = new SimpleVertex[vertexCountchessBoard];

	for (int i = 0; i<vertexCountchessBoard; i++)
	{
		fin >> verticeschessBoard[i].Pos.x >> verticeschessBoard[i].Pos.y >> verticeschessBoard[i].Pos.z;
		fin >> verticeschessBoard[i].Tex.x >> verticeschessBoard[i].Tex.y;
		fin >> verticeschessBoard[i].Normal.x >> verticeschessBoard[i].Normal.y >> verticeschessBoard[i].Normal.z;
	}	
	fin.close();

	// Buradaki genel setlemelerin bazýlarý aþaðýdaki diðer setlemelerde kullanýlmayabilir.
	// Diðer setlemeler burdaki gibi uzun deðil, mesela piyon için bd.ByteWidth... ile  if(FAILED(hr))... arasý yani sadece 5 satýr. 
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex)* vertexCountchessBoard;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = verticeschessBoard;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_chessBoard);
	if (FAILED(hr))     return hr;
	/////////////////

	// Piyonu yükle
	fin.open("piyon.txt");
	fin >> vertexCountPiyon;
	verticesPiyon = new SimpleVertex[vertexCountPiyon];

	for(int i=0; i<vertexCountPiyon; i++)
	{
		fin >> verticesPiyon[i].Pos.x >> verticesPiyon[i].Pos.y >> verticesPiyon[i].Pos.z;
		fin >> verticesPiyon[i].Tex.x >> verticesPiyon[i].Tex.y;
		fin >> verticesPiyon[i].Normal.x >> verticesPiyon[i].Normal.y >> verticesPiyon[i].Normal.z;
	}	
	fin.close();

	bd.ByteWidth = sizeof( SimpleVertex ) * vertexCountPiyon;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = verticesPiyon;
	hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer_Piyon);
	if( FAILED( hr ) )     return hr;
	//-------------------------------------------------------------------------------------------------

	// Atý yükle
	fin.open("at.txt");
	fin >> vertexCountAt;
	verticesAt = new SimpleVertex[vertexCountAt];

	for(int i=0; i<vertexCountAt; i++)
	{
		fin >> verticesAt[i].Pos.x >> verticesAt[i].Pos.y >> verticesAt[i].Pos.z;
		fin >> verticesAt[i].Tex.x >> verticesAt[i].Tex.y;
		fin >> verticesAt[i].Normal.x >> verticesAt[i].Normal.y >> verticesAt[i].Normal.z;
	}	
	fin.close();

	bd.ByteWidth = sizeof( SimpleVertex ) * vertexCountAt;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = verticesAt;
	hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer_At);
	if( FAILED( hr ) )     return hr;
	//-------------------------------------------------------------------------------------------------

	// Fili yükle
	fin.open("fil.txt");
	fin >> vertexCountFil;
	verticesFil = new SimpleVertex[vertexCountFil];

	for(int i=0; i<vertexCountFil; i++)
	{
		fin >> verticesFil[i].Pos.x >> verticesFil[i].Pos.y >> verticesFil[i].Pos.z;
		fin >> verticesFil[i].Tex.x >> verticesFil[i].Tex.y;
		fin >> verticesFil[i].Normal.x >> verticesFil[i].Normal.y >> verticesFil[i].Normal.z;
	}	
	fin.close();

	bd.ByteWidth = sizeof( SimpleVertex ) * vertexCountFil;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = verticesFil;
	hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer_Fil);
	if( FAILED( hr ) )     return hr;
	//-------------------------------------------------------------------------------------------------

	//Kaleyi YÜKLE
	fin.open("kale.txt");
	fin >> vertexCountKale;
	verticesKale= new SimpleVertex[vertexCountKale];

	for(int i=0; i<vertexCountKale; i++)
	{
		fin >> verticesKale[i].Pos.x >> verticesKale[i].Pos.y >> verticesKale[i].Pos.z;
		fin >> verticesKale[i].Tex.x >> verticesKale[i].Tex.y;
		fin >> verticesKale[i].Normal.x >> verticesKale[i].Normal.y >> verticesKale[i].Normal.z;
	}	
	fin.close();

	bd.ByteWidth = sizeof( SimpleVertex ) * vertexCountKale;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = verticesKale;
	hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer_Kale);
	if( FAILED( hr ) )     return hr;
	//-------------------------------------------------------------------------------------------------

	//Veziri YÜKLE
	fin.open("vezir.txt");
	fin >> vertexCountVezir;
	verticesVezir = new SimpleVertex[vertexCountVezir];

	for(int i=0; i<vertexCountVezir; i++)
	{
		fin >> verticesVezir[i].Pos.x >> verticesVezir[i].Pos.y >> verticesVezir[i].Pos.z;
		fin >> verticesVezir[i].Tex.x >> verticesVezir[i].Tex.y;
		fin >> verticesVezir[i].Normal.x >> verticesVezir[i].Normal.y >> verticesVezir[i].Normal.z;
	}	
	fin.close();

	bd.ByteWidth = sizeof( SimpleVertex ) * vertexCountVezir;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = verticesVezir;
	hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer_Vezir);
	if( FAILED( hr ) )     return hr;
	//-------------------------------------------------------------------------------------------------

	// Þahý YÜKLE
	fin.open("sah.txt");
	fin >> vertexCountSah;
	verticesSah = new SimpleVertex[vertexCountSah];

	for(int i=0; i<vertexCountSah; i++)
	{
		fin >> verticesSah[i].Pos.x >> verticesSah[i].Pos.y >> verticesSah[i].Pos.z;
		fin >> verticesSah[i].Tex.x >> verticesSah[i].Tex.y;
		fin >> verticesSah[i].Normal.x >> verticesSah[i].Normal.y >> verticesSah[i].Normal.z;
	}	
	fin.close();

	bd.ByteWidth = sizeof( SimpleVertex ) * vertexCountSah;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = verticesSah;
	hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer_Sah);
	if( FAILED( hr ) )     return hr;
	//-------------------------------------------------------------------------------------------------

	fin.open("Light.txt");
	int vertexCountlight;
	fin >> vertexCountlight;
	verticesLight = new SimpleVertex[vertexCountlight];

	for (int i = 0; i<vertexCountlight; i++)
	{
		fin >> verticesLight[i].Pos.x >> verticesLight[i].Pos.y >> verticesLight[i].Pos.z;
		fin >> verticesLight[i].Tex.x >> verticesLight[i].Tex.y;
		fin >> verticesLight[i].Normal.x >> verticesLight[i].Normal.y >> verticesLight[i].Normal.z;
	}

	fin.close();

	bd.ByteWidth = sizeof(SimpleVertex)* vertexCountlight;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = verticesLight;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_Light);
	if (FAILED(hr))     return hr;

	//Pointer yukle
	fin.open("pointer.txt");

	fin >> vertexCountpointer;
	verticespointer = new SimpleVertex[vertexCountpointer];

	for (int i = 0; i<vertexCountpointer; i++)
	{
		fin >> verticespointer[i].Pos.x >> verticespointer[i].Pos.y >> verticespointer[i].Pos.z;
		fin >> verticespointer[i].Tex.x >> verticespointer[i].Tex.y;
		fin >> verticespointer[i].Normal.x >> verticespointer[i].Normal.y >> verticespointer[i].Normal.z;
	}

	fin.close();

	bd.ByteWidth = sizeof(SimpleVertex) * vertexCountpointer;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = verticespointer;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_Pointer);
	if (FAILED(hr))     return hr;
	//-------------------------------------------------------------------------------------------------

	//Hamleler icin pointer yukle
	fin.open("pointer.txt");

	fin >> vertexCountpointer;
	verticesPointerHamle = new SimpleVertex[vertexCountpointer];

	for (int i = 0; i<vertexCountpointer; i++)
	{
		fin >> verticesPointerHamle[i].Pos.x >> verticesPointerHamle[i].Pos.y >> verticesPointerHamle[i].Pos.z;
		fin >> verticesPointerHamle[i].Tex.x >> verticesPointerHamle[i].Tex.y;
		fin >> verticesPointerHamle[i].Normal.x >> verticesPointerHamle[i].Normal.y >> verticesPointerHamle[i].Normal.z;
	}

	fin.close();

	bd.ByteWidth = sizeof(SimpleVertex) * vertexCountpointer;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = verticesPointerHamle;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_PointerHamle);
	if (FAILED(hr))     return hr;
	//-------------------------------------------------------------------------------------------------

	fin.open("CrossHair.txt");

	fin >> vertexCountcrosshair;
	verticesCrossHair = new SimpleVertex[vertexCountcrosshair];

	for (int i = 0; i<vertexCountcrosshair; i++)
	{
		fin >> verticesCrossHair[i].Pos.x >> verticesCrossHair[i].Pos.y >> verticesCrossHair[i].Pos.z;
		fin >> verticesCrossHair[i].Tex.x >> verticesCrossHair[i].Tex.y;
		fin >> verticesCrossHair[i].Normal.x >> verticesCrossHair[i].Normal.y >> verticesCrossHair[i].Normal.z;
	}
	
	fin.close();

	bd.ByteWidth = sizeof(SimpleVertex) * vertexCountcrosshair;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = verticesCrossHair;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer_CrossHair );
    if( FAILED( hr ) )     return hr;

    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

 
	// Create the constant buffers
    ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pConstantBuffer );
    if( FAILED( hr ) )	return hr;
    
    bd.ByteWidth = sizeof(ConstantBuffer);
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pConstantBuffer );
    if( FAILED( hr ) )	return hr;
    
    bd.ByteWidth = sizeof(ConstantBuffer);
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pConstantBuffer );
    if( FAILED( hr ) )  return hr;

	// isaret edilen kare için dolgu beyaz
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_White.jpg", NULL, NULL, &g_pTexturepointerWhite, NULL);
	if (FAILED(hr))	return hr;

	// isaret edilen kare için dolgu siyah
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_Black.jpg", NULL, NULL, &g_pTexturepointerBlack, NULL);
	if (FAILED(hr))	return hr;

	// Hamle icin secilen beyaz kareler
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_White_2.jpg", NULL, NULL, &g_pTexturepointerWhite_2, NULL);
	if (FAILED(hr))	return hr;

	// Hamle icin secilen siyah kareler
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_Black_2.jpg", NULL, NULL, &g_pTexturepointerBlack_2, NULL);
	if (FAILED(hr))	return hr;

	// Yenebilecek hamleler icin beyaz
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_WhiteRed.jpg", NULL, NULL, &g_pTexturepointerWhiteRed, NULL);
	if (FAILED(hr))	return hr;

	// Yenebilecek hamleler icin siyah
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_BlackRed.jpg", NULL, NULL, &g_pTexturepointerBlackRed, NULL);
	if (FAILED(hr))	return hr;

	// Mevcut hamleler icin beyaz
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_White_3.jpg", NULL, NULL, &g_pTexturepointerWhite_3, NULL);
	if (FAILED(hr))	return hr;

	// Mevcut hamleler icin siyah
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_Black_3.jpg", NULL, NULL, &g_pTexturepointerBlack_3, NULL);
	if (FAILED(hr))	return hr;

	// Yenebilecek hamleler icin beyaz
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_WhiteRed_2.jpg", NULL, NULL, &g_pTexturepointerWhiteRed_2, NULL);
	if (FAILED(hr))	return hr;

	// Yenebilecek hamleler icin siyah
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_BlackRed_2.jpg", NULL, NULL, &g_pTexturepointerBlackRed_2, NULL);
	if (FAILED(hr))	return hr;

	// Secilen tas icin beyaz
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_White_4.jpg", NULL, NULL, &g_pTexturepointerWhite_4, NULL);
	if (FAILED(hr))	return hr;

	// Secilen tas icin siyah
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_Black_4.jpg", NULL, NULL, &g_pTexturepointerBlack_4, NULL);
	if (FAILED(hr))	return hr;

	// Hamlesi olan taslar ýcýn beyaz
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_White_5.jpg", NULL, NULL, &g_pTexturepointerWhite_5, NULL);
	if (FAILED(hr))	return hr;

	// hamlesi olan taslar ýcýn siyah
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_Black_5.jpg", NULL, NULL, &g_pTexturepointerBlack_5, NULL);
	if (FAILED(hr))	return hr;

	// Hamlesi olan taslar ýcýn uzerýne gelýndýgýnde beyaz
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_White_6.jpg", NULL, NULL, &g_pTexturepointerWhite_6, NULL);
	if (FAILED(hr))	return hr;

	// hamlesi olan taslar ýcýn uzerýne gelýndýgýnde siyah
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"pointer_Black_6.jpg", NULL, NULL, &g_pTexturepointerBlack_6, NULL);
	if (FAILED(hr))	return hr;
	
    // Load the Texture chessBoard
    hr = D3DX11CreateShaderResourceViewFromFile( g_pd3dDevice, L"chessBoard.jpg", NULL, NULL, &g_pTexturechessBoard, NULL );
    if( FAILED( hr ) )	return hr;

    // Zemin kaplamasý icin
    hr = D3DX11CreateShaderResourceViewFromFile( g_pd3dDevice, L"chessBoard_2.jpg", NULL, NULL, &g_pTexturechessBoard_2, NULL );
    if( FAILED( hr ) )	return hr;

    // Create the sample state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState( &sampDesc, &g_pSamplerLinear );
    if( FAILED( hr ) )	return hr;

    // Initialize the world matrices
    g_World			= XMMatrixIdentity();

    // Initialize the view matrix
    Eye = XMVectorSet( 0.0f, 25.0f, -45.0f, 0.0f );
    At = XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f );
    Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    g_View = XMMatrixLookAtLH( Eye, At, Up );

    // Initialize the projection matrix
    g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, width / (FLOAT)height, 0.01f, 1000.0f );
    
    cBuffer.mProjection = XMMatrixTranspose( g_Projection );
    g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );


	Ro = Eye;
	Rd = XMVector3Normalize(At - Eye);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if( g_pImmediateContext )		g_pImmediateContext->ClearState();
    if( g_pSamplerLinear )			g_pSamplerLinear->Release();
    //bellek alanlarý serbest býrakýldý
	//-------------------------------------------------------------------------------
    if( g_pTexturechessBoard )		g_pTexturechessBoard->Release();
    if( g_pConstantBuffer )			g_pConstantBuffer->Release();
	if( g_pVertexBuffer_CrossHair ) g_pVertexBuffer_CrossHair->Release();

	//bellek alanlarý serbest býrakýldý
	//-------------------------------------------------------------------------------
	if( g_pVertexBuffer_Piyon)		g_pVertexBuffer_Piyon->Release();
	if (g_pVertexBuffer_At)			g_pVertexBuffer_At->Release();
	if (g_pVertexBuffer_Fil)		g_pVertexBuffer_Fil->Release();
	if (g_pVertexBuffer_Kale)		g_pVertexBuffer_Kale->Release();
	if (g_pVertexBuffer_Vezir)		g_pVertexBuffer_Vezir->Release();
	if (g_pVertexBuffer_Sah)		g_pVertexBuffer_Sah->Release();
	//-------------------------------------------------------------------------------

	if( g_pVertexBuffer_Light )		g_pVertexBuffer_Light->Release();
    if( g_pIndexBuffer )			g_pIndexBuffer->Release();
    if( g_pVertexLayout )			g_pVertexLayout->Release();
    if( g_pVertexShader )			g_pVertexShader->Release();
    if( g_pPixelShader )			g_pPixelShader->Release();
	if( g_pPixelShader )			g_pPixelShader_Solid->Release();
    if( g_pDepthStencil )			g_pDepthStencil->Release();
    if( g_pDepthStencilView )		g_pDepthStencilView->Release();
    if( g_pRenderTargetView )		g_pRenderTargetView->Release();
    if( g_pSwapChain )				g_pSwapChain->Release();
    if( g_pImmediateContext )		g_pImmediateContext->Release();
    if( g_pd3dDevice )				g_pd3dDevice->Release();
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render(double time)
{
    // Update our time
	//rot += 1.0f * time;
	//if(rot > 6.28f) rot = 0.0f;

	//Burada Taþýn hareket hýzý ne olmalý onun hesabý yapýlýyor.(Render hýzý ekran kartýna baglý olarak degiþtigi icin hereketýn tum PC lerde hemen hemen ayný hýzda olmasý ýcýn bunu yapýyoruz)
	tasHareketHiz = (time*10)/2;
	//-------------------------------------------

	Ro = Eye;
	Rd = XMVector3Normalize(At - Eye);

    // Clear the back buffer
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );

    // Clear the depth buffer to 1.0 (max depth)
    g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );


    cBuffer.mView = XMMatrixTranspose( g_View );
	XMStoreFloat4( &cBuffer.vEyePos, Eye );
    g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );

    XMFLOAT4 vLightPos[2] =
    {
        XMFLOAT4( 0.0f,  0.0f,   0.0f,  1.0f ),
		XMFLOAT4( 0.0f,  0.0f,   0.0f,  1.0f )
    };

    XMFLOAT4 vLightColors[2] =
    {
        XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ),
		XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f )
    };

	cBuffer.vLightColor[0] = vLightColors[0];
	cBuffer.vLightColor[1] = vLightColors[1];
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );

	UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Light, &stride, &offset );

	XMMATRIX mRotate;
	XMMATRIX mTranslate1, mTranslate2, mTranslate3;

	mTranslate1	= XMMatrixTranslation( -5.0f, 0.0f, 0.0f );
	mTranslate2	= XMMatrixTranslation( 0.0f, 50.0f, 50.0f );
	mRotate 	= XMMatrixRotationY( rot );
	
	XMVECTOR vLightPos0 = XMLoadFloat4( &vLightPos[0] );
	vLightPos0 = XMVector3Transform( vLightPos0, mTranslate1 );
	vLightPos0 = XMVector3Transform( vLightPos0, mRotate );
	vLightPos0 = XMVector3Transform( vLightPos0, mTranslate2 );
	XMStoreFloat4( &vLightPos[0], vLightPos0 );

	cBuffer.mWorld = XMMatrixTranspose(mTranslate1 * mRotate * mTranslate2); 
	cBuffer.vLightPos[0] = vLightPos[0];
	cBuffer.vMeshColor = vLightColors[0];
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );

	// Render White Light
	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
	g_pImmediateContext->PSSetShader( g_pPixelShader_Solid, NULL, 0 );
	g_pImmediateContext->PSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
	g_pImmediateContext->Draw(vertexCountlight, 0);

	mTranslate1	= XMMatrixTranslation(  5.0f, 0.0f, 0.0f );
	mTranslate3	= XMMatrixTranslation(  0.0f, 50.0f, -50.0f );

	XMVECTOR vLightPos1 = XMLoadFloat4( &vLightPos[1] );
	vLightPos1 = XMVector3Transform( vLightPos1, mTranslate1 );
	vLightPos1 = XMVector3Transform( vLightPos1, mRotate );
	vLightPos1 = XMVector3Transform( vLightPos1, mTranslate3 );
	XMStoreFloat4( &vLightPos[1], vLightPos1 );

	cBuffer.mWorld = XMMatrixTranspose(mTranslate1 * mRotate * mTranslate3); 
	cBuffer.vLightPos[1] = vLightPos[1];
	cBuffer.vMeshColor = vLightColors[1];
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
	/////////////////////////////////////////////////////

	// Render Yellow Light
	g_pImmediateContext->Draw(vertexCountlight, 0);
	

	// Render CrossHair
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_CrossHair, &stride, &offset);
	XMFLOAT4 sPosition; 
	XMStoreFloat4( &sPosition, crosshairPosition );
	XMMATRIX mTranslate	= XMMatrixTranslation( sPosition.x, sPosition.y, sPosition.z );
    cBuffer.mWorld = XMMatrixTranspose( mTranslate );	
	cBuffer.vMeshColor = XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ); 
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
	g_pImmediateContext->PSSetShader( g_pPixelShader_Solid, NULL, 0 );
	g_pImmediateContext->Draw(vertexCountcrosshair, 0);

	g_World = XMMatrixIdentity();
    cBuffer.mWorld = XMMatrixTranspose( g_World );
    g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );  
	////////////////////////////////////////////////////////////

    // Render ChessBoard
	g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_chessBoard, &stride, &offset );
	cBuffer.vMeshColor = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ); 

	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
    g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
    g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
    g_pImmediateContext->PSSetShader( g_pPixelShader_Textured, NULL, 0 );
	g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
	g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pTexturechessBoard );
	g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerLinear );
	g_pImmediateContext->Draw(vertexCountchessBoard, 0);
	///////////////////////////////////////////////////////////

    // Zemin kaplama
	for(int i = -40; i<=40; i+=40){
		for(int j = -40; j<=40;j+=40){
			if(!(i==0 && j==0)){
				g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Pointer, &stride, &offset );
				cBuffer.vMeshColor = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ); 

				XMMATRIX mScaleChessBoard= XMMatrixScaling(8,1,8);
				XMMATRIX mTranslateChessBoard= XMMatrixTranslation(i, 0.0f, j);
				cBuffer.mWorld = XMMatrixTranspose(mScaleChessBoard*mTranslateChessBoard);
				g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
				g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
				g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
				g_pImmediateContext->PSSetShader( g_pPixelShader_Textured, NULL, 0 );
				g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
				g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pTexturechessBoard_2 );
				g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerLinear );
				g_pImmediateContext->Draw(vertexCountchessBoard, 0);
			}
		}
	}
	//----------------------------------------------------------------------------------------------

	// Render Pointer
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_Pointer, &stride, &offset);
	cBuffer.vMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR translatePointer = testIntersection(Ro, Rd, verticeschessBoard, vertexCountchessBoard, false, cBuffer.mWorld);
	XMFLOAT3 translateP;
	XMStoreFloat3(&translateP, translatePointer);

	int secilenX, secilenZ;
	if (translateP.x != 0.0F)
	{
		secilenX = (translateP.x + 20) / 5.0f;
		secilenZ = (translateP.z + 20) / 5.0f;
		int renkDurum;
		if(oyunSirasi){renkDurum = 1;}
		else{renkDurum = 2;}

		//burada siyah mý beyazmý kontrolunu degiþtim.
		if (((secilenX + secilenZ) % 2) != 0 ){
			if(renderPointerHamle[secilenX][secilenZ] == 1){
				g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerWhite_3);
			}
			else if(renderPointerHamle[secilenX][secilenZ] == 2){
				g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerWhiteRed_2);
			}
			else if(renderPointerHamle[secilenX][secilenZ] == 3){
				g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerWhite_4);
			}
			else if(renderPointerHamle[secilenX][secilenZ] == 5){
				g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerWhite_6);
			}
			else if(renderPointerHamle[secilenX][secilenZ] == 0){
				g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerWhite);
			}
		}
		else{
			if(renderPointerHamle[secilenX][secilenZ] == 1){
				g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerBlack_3);
			}
			else if(renderPointerHamle[secilenX][secilenZ] == 2){
				g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerBlackRed_2);
			}
			else if(renderPointerHamle[secilenX][secilenZ] == 3){
				g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerBlack_4);
			}
			else if(renderPointerHamle[secilenX][secilenZ] == 5){
				g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerBlack_6);
			}
			else if(renderPointerHamle[secilenX][secilenZ] == 0){
				g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerBlack);
			}
		}

		XMMATRIX mTranslatePointer = XMMatrixTranslation(translateP.x, 0, translateP.z);
		cBuffer.mWorld = XMMatrixTranspose(mTranslatePointer);
		g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0);
		g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
		g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pConstantBuffer);
		g_pImmediateContext->PSSetShader(g_pPixelShader_Textured, NULL, 0);
		g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pConstantBuffer);
		g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
		g_pImmediateContext->Draw(vertexCountpointer, 0);
	}
	//-------------------------------------------------------------------------------------------------

	// Render Hamle pointer larý
	if(renderPointerHamleDurum){
		for (int i = 0; i<8; i++){
			for(int j = 0; j<8; j++){
				if(renderPointerHamle[i][j] != 0)
				{
					g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_PointerHamle, &stride, &offset );
					cBuffer.vMeshColor = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ); // Beyaz

					//burada siyah mý beyazmý kontrolunu degiþtim.
					if (((i + j) % 2) != 0){			
						//yenecek tas varmý yok mu ona gore pointer secilir
						if(renderPointerHamle[i][j] == 1){
							g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerWhite_2);
						}
						else if(renderPointerHamle[i][j] == 2){
							g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerWhiteRed);
						}
						else if(renderPointerHamle[i][j] == 3){
							g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerWhite_4);
						}
						else if(renderPointerHamle[i][j] == 5){
							g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerWhite_5);
						}
					}
					else{
						//yenecek tas varmý yok mu ona gore pointer secilir
						if(renderPointerHamle[i][j] == 1){
							g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerBlack_2);
						}
						else if(renderPointerHamle[i][j] == 2){
							g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerBlackRed);
						}
						else if(renderPointerHamle[i][j] == 3){
							g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerBlack_4);
						}
						else if(renderPointerHamle[i][j] == 5){
							g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerBlack_5);
						}
					}

					XMMATRIX mTranslatePointerHamle = XMMatrixTranslation(satrancTahtasiX[i], 0.0f, satrancTahtasiZ[j]);
					cBuffer.mWorld = XMMatrixTranspose(mTranslatePointerHamle);
					g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0);
					g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
					g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pConstantBuffer);
					g_pImmediateContext->PSSetShader(g_pPixelShader_Textured, NULL, 0);
					g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pConstantBuffer);
					g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
					g_pImmediateContext->Draw(vertexCountpointer, 0);
				}
			}
		}
	}
	//------------------------------------------------------------------------------------------------

	// Tehlikedeki sah'ý kýrmýzý olarak goster
	for(int i = 0; i<2; i++){
		if(sahTehlikeDurumRender[i])
		{
			g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_PointerHamle, &stride, &offset );
			cBuffer.vMeshColor = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ); // Beyaz

			//burada siyah mý beyazmý kontrolunu degiþtim.
			int sahX;
			int sahZ;
			if(i == 0){
				sahX = (sahBeyazKonum[0] + 17.5) / 5;
				sahZ = (sahBeyazKonum[2] + 17.5) / 5;
			}
			else{
				sahX = (sahSiyahKonum[0] + 17.5) / 5;
				sahZ = (sahSiyahKonum[2] + 17.5) / 5;
			}

			if (((sahX + sahZ) % 2) != 0){
				g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerWhiteRed);
			}
			else{
				g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexturepointerBlackRed);
			}

			XMMATRIX mTranslatePointerHamle = XMMatrixTranslation(satrancTahtasiX[sahX], 0.0f, satrancTahtasiZ[sahZ]);
			cBuffer.mWorld = XMMatrixTranspose(mTranslatePointerHamle);
			g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0);
			g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
			g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pConstantBuffer);
			g_pImmediateContext->PSSetShader(g_pPixelShader_Textured, NULL, 0);
			g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pConstantBuffer);
			g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
			g_pImmediateContext->Draw(vertexCountpointer, 0);
		}
	}
	//-------------------------------------------------------------------------------------------------

	//Burada taslarýn anýmasyonu gerceklendý
	//--------------------------------------
	TasHareket();
	//--------------------------------------

	// Render Beyaz Piyonlar
	int x,z;
	for (int i = 0; i<8; i++){
		x = (piyonBeyazKonumHareket[i][0] + 17.5)/5;
		z = (piyonBeyazKonumHareket[i][2] + 17.5)/5;

		//Burada piyon vezir olmussa vezir seklýnde ekranda gorunmesý gerceklestýrýldý
		if((x>=0 && x<=7) && (z>=0 && z<=7)){
			if(oyunDurum[x][z][0] == 5)
				g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Vezir, &stride, &offset );
			else
				g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Piyon, &stride, &offset );
		}
		else
			g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Piyon, &stride, &offset );
		//----------------------------------------------------------------------------

		if(renderPointerHamle[x][z] == 2 && !oyunSirasi && (x>=0 && x<=7 && z>=0 && z<=7)){
			cBuffer.vMeshColor = RedColor; // Kýrmýzý
		}
		else{
			cBuffer.vMeshColor = WhiteColor; // Beyaz
		}

		XMMATRIX mTranslateBeyazPiyon = XMMatrixTranslation(piyonBeyazKonum[i][0], piyonBeyazKonum[i][1], piyonBeyazKonum[i][2]);
		g_World = mTranslateBeyazPiyon;
		cBuffer.mWorld = XMMatrixTranspose( g_World );
		g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
		g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
		g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
		g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		
		//piyon Vezir olmussa vezirin vertexCount u draw a verilir
		if((x>=0 && x<=7) && (z>=0 && z<=7)){
			if(oyunDurum[x][z][0] == 5)
				g_pImmediateContext->Draw(vertexCountVezir, 0 );
			else
				g_pImmediateContext->Draw(vertexCountPiyon, 0 );
		}
		else
			g_pImmediateContext->Draw(vertexCountPiyon, 0 );
		//--------------------------------------------------------
	}
	//-------------------------------------------------------------------------------------------------

	// Render Beyaz Atlar
	for (int i = 0; i<2; i++){
		g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_At, &stride, &offset );
		
		x = (atBeyazKonumHareket[i][0] + 17.5)/5;
		z = (atBeyazKonumHareket[i][2] + 17.5)/5;
		if(renderPointerHamle[x][z] == 2 && !oyunSirasi && (x>=0 && x<=7 && z>=0 && z<=7)){
			cBuffer.vMeshColor = RedColor; // Kýrmýzý
		}
		else{
			cBuffer.vMeshColor = WhiteColor; // Beyaz
		}

		XMMATRIX mRotationBeyazAt = XMMatrixRotationY(3.14);//Burada beyaz atlarý 180 derece  cevirdik duz gozukmelerý ýcýn
		XMMATRIX mTranslateBeyazAt = XMMatrixTranslation( atBeyazKonum[i][0], atBeyazKonum[i][1], atBeyazKonum[i][2]);
		g_World = mRotationBeyazAt*mTranslateBeyazAt;
		cBuffer.mWorld = XMMatrixTranspose( g_World );
		g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
		g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
		g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
		g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		g_pImmediateContext->Draw(vertexCountAt, 0 );
	}
	//-------------------------------------------------------------------------------------------------

	// Render Beyaz Filler
	for (int i = 0; i<2; i++){
		g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Fil, &stride, &offset );
		
		x = (filBeyazKonumHareket[i][0] + 17.5)/5;
		z = (filBeyazKonumHareket[i][2] + 17.5)/5;
		if(renderPointerHamle[x][z] == 2 && !oyunSirasi && (x>=0 && x<=7 && z>=0 && z<=7)){
			cBuffer.vMeshColor = RedColor; // Kýrmýzý
		}
		else{
			cBuffer.vMeshColor = WhiteColor; // Beyaz
		}

		XMMATRIX mTranslateBeyazFil = XMMatrixTranslation( filBeyazKonum[i][0], filBeyazKonum[i][1], filBeyazKonum[i][2]);
		g_World = mTranslateBeyazFil;
		cBuffer.mWorld = XMMatrixTranspose( g_World );
		g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
		g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
		g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
		g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		g_pImmediateContext->Draw(vertexCountFil, 0 );
	}
	//-------------------------------------------------------------------------------------------------

	// Render Beyaz Kaleler
	for (int i = 0; i<2; i++){
		g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Kale, &stride, &offset );
		
		x = (kaleBeyazKonumHareket[i][0] + 17.5)/5;
		z = (kaleBeyazKonumHareket[i][2] + 17.5)/5;
		if(renderPointerHamle[x][z] == 2 && !oyunSirasi && (x>=0 && x<=7 && z>=0 && z<=7)){
			cBuffer.vMeshColor = RedColor; // Kýrmýzý
		}
		else{
			cBuffer.vMeshColor = WhiteColor; // Beyaz
		}

		XMMATRIX mTranslateBeyazKale = XMMatrixTranslation( kaleBeyazKonum[i][0], kaleBeyazKonum[i][1], kaleBeyazKonum[i][2]);
		g_World = mTranslateBeyazKale;
		cBuffer.mWorld = XMMatrixTranspose( g_World );
		g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
		g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
		g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
		g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		g_pImmediateContext->Draw(vertexCountKale, 0 );
	}
	//-------------------------------------------------------------------------------------------------

	// Render Beyaz Vezir
	g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Vezir, &stride, &offset );
	
	x = (vezirBeyazKonumHareket[0] + 17.5)/5;
	z = (vezirBeyazKonumHareket[2] + 17.5)/5;
	if(renderPointerHamle[x][z] == 2 && !oyunSirasi && (x>=0 && x<=7 && z>=0 && z<=7)){
		cBuffer.vMeshColor = RedColor; // Kýrmýzý
	}
	else{
		cBuffer.vMeshColor = WhiteColor; // Beyaz
	}

	XMMATRIX mTranslateBeyazVezir = XMMatrixTranslation( vezirBeyazKonum[0], vezirBeyazKonum[1], vezirBeyazKonum[2]);
	g_World = mTranslateBeyazVezir;
	cBuffer.mWorld = XMMatrixTranspose( g_World );
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
	g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
	g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
	g_pImmediateContext->Draw(vertexCountVezir, 0 );
	//-------------------------------------------------------------------------------------------------

	// Render Beyaz Þah
	g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Sah, &stride, &offset );

	x = (sahBeyazKonumHareket[0] + 17.5)/5;
	z = (sahBeyazKonumHareket[2] + 17.5)/5;
	if(sahTehlikeDurumRender[0]){
		cBuffer.vMeshColor = RedColor; // Kýrmýzý
	}
	else{
		cBuffer.vMeshColor = WhiteColor; // Beyaz
	}

	XMMATRIX mTranslateBeyazSah = XMMatrixTranslation( sahBeyazKonum[0], sahBeyazKonum[1], sahBeyazKonum[2]);
	g_World = mTranslateBeyazSah;
	cBuffer.mWorld = XMMatrixTranspose( g_World );
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
	g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
	g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
	g_pImmediateContext->Draw(vertexCountSah, 0 );
	//-------------------------------------------------------------------------------------------------

	// Render Siyah Piyonlar
	for (int i = 0; i<8; i++){
		x = (piyonSiyahKonumHareket[i][0] + 17.5)/5;
		z = (piyonSiyahKonumHareket[i][2] + 17.5)/5;

		//Burada piyon vezir olmussa vezir seklýnde ekranda gorunmesý gerceklestýrýldý
		if((x>=0 && x<=7) && (z>=0 && z<=7)){
			if(oyunDurum[x][z][0] == 5)
				g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Vezir, &stride, &offset );
			else
				g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Piyon, &stride, &offset );
		}
		else
			g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Piyon, &stride, &offset );
		//----------------------------------------------------------------------------

		if(renderPointerHamle[x][z] == 2 && oyunSirasi && (x>=0 && x<=7 && z>=0 && z<=7)){
			cBuffer.vMeshColor = RedColor; // Kýrmýzý
		}
		else{
			cBuffer.vMeshColor = BlackColor; // Siyah
		}

		XMMATRIX mTranslateSiyahPiyon = XMMatrixTranslation( piyonSiyahKonum[i][0], piyonSiyahKonum[i][1], piyonSiyahKonum[i][2]);
		g_World = mTranslateSiyahPiyon;
		cBuffer.mWorld = XMMatrixTranspose( g_World );
		g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
		g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
		g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
		g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );

		//piyon Vezir olmussa vezirin vertexCount u draw a verilir
		if((x>=0 && x<=7) && (z>=0 && z<=7)){
			if(oyunDurum[x][z][0] == 5)
				g_pImmediateContext->Draw(vertexCountVezir, 0 );
			else
				g_pImmediateContext->Draw(vertexCountPiyon, 0 );
		}
		else
			g_pImmediateContext->Draw(vertexCountPiyon, 0 );
		//--------------------------------------------------------
	}
	//-------------------------------------------------------------------------------------------------

	// Render Siyah Atlar
	for (int i = 0; i<2; i++){
		g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_At, &stride, &offset );

		x = (atSiyahKonumHareket[i][0] + 17.5)/5;
		z = (atSiyahKonumHareket[i][2] + 17.5)/5;
		if(renderPointerHamle[x][z] == 2 && oyunSirasi && (x>=0 && x<=7 && z>=0 && z<=7)){
			cBuffer.vMeshColor = RedColor; // Kýrmýzý
		}
		else{
			cBuffer.vMeshColor = BlackColor; // Siyah
		}

		XMMATRIX mTranslateSiyahAt = XMMatrixTranslation( atSiyahKonum[i][0], atSiyahKonum[i][1], atSiyahKonum[i][2]);
		g_World = mTranslateSiyahAt;
		cBuffer.mWorld = XMMatrixTranspose( g_World );
		g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
		g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
		g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
		g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		g_pImmediateContext->Draw(vertexCountAt, 0 );
	}
	//-------------------------------------------------------------------------------------------------

	// Render Siyah Filler
	for (int i = 0; i<2; i++){
		g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Fil, &stride, &offset );
		
		x = (filSiyahKonumHareket[i][0] + 17.5)/5;
		z = (filSiyahKonumHareket[i][2] + 17.5)/5;
		if(renderPointerHamle[x][z] == 2 && oyunSirasi && (x>=0 && x<=7 && z>=0 && z<=7)){
			cBuffer.vMeshColor = RedColor; // Kýrmýzý
		}
		else{
			cBuffer.vMeshColor = BlackColor; // Siyah
		}

		XMMATRIX mTranslateSiyahFil = XMMatrixTranslation( filSiyahKonum[i][0], filSiyahKonum[i][1], filSiyahKonum[i][2]);
		g_World = mTranslateSiyahFil;
		cBuffer.mWorld = XMMatrixTranspose( g_World );
		g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
		g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
		g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
		g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		g_pImmediateContext->Draw(vertexCountFil, 0 );
	}
	//-------------------------------------------------------------------------------------------------

	// Render Siyah Kaleler
	for (int i = 0; i<2; i++){
		g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Kale, &stride, &offset );
		
		x = (kaleSiyahKonumHareket[i][0] + 17.5)/5;
		z = (kaleSiyahKonumHareket[i][2] + 17.5)/5;
		if(renderPointerHamle[x][z] == 2 && oyunSirasi && (x>=0 && x<=7 && z>=0 && z<=7)){
			cBuffer.vMeshColor = RedColor; // Kýrmýzý
		}
		else{
			cBuffer.vMeshColor = BlackColor; // Siyah
		}

		XMMATRIX mTranslateSiyahKale = XMMatrixTranslation( kaleSiyahKonum[i][0], kaleSiyahKonum[i][1], kaleSiyahKonum[i][2]);
		g_World = mTranslateSiyahKale;
		cBuffer.mWorld = XMMatrixTranspose( g_World );
		g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
		g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
		g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
		g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
		g_pImmediateContext->Draw(vertexCountKale, 0 );
	}
	//-------------------------------------------------------------------------------------------------

	// Render Siyah Vezir
	g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Vezir, &stride, &offset );
		
	x = (vezirSiyahKonumHareket[0] + 17.5)/5;
	z = (vezirSiyahKonumHareket[2] + 17.5)/5;
	if(renderPointerHamle[x][z] == 2 && oyunSirasi && (x>=0 && x<=7 && z>=0 && z<=7)){
		cBuffer.vMeshColor = RedColor; // Kýrmýzý
	}
	else{
		cBuffer.vMeshColor = BlackColor; // Siyah
	}

	XMMATRIX mTranslateSiyahVezir = XMMatrixTranslation( vezirSiyahKonum[0], vezirSiyahKonum[1], vezirSiyahKonum[2]);
	g_World = mTranslateSiyahVezir;
	cBuffer.mWorld = XMMatrixTranspose( g_World );
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
	g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
	g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
	g_pImmediateContext->Draw(vertexCountVezir, 0 );
	//-------------------------------------------------------------------------------------------------

	// Render Siyah Þah
	g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Sah, &stride, &offset );
	
	x = (sahSiyahKonumHareket[0] + 17.5)/5;
	z = (sahSiyahKonumHareket[2] + 17.5)/5;
	if(sahTehlikeDurumRender[1]){
		cBuffer.vMeshColor = RedColor; // Kýrmýzý
	}
	else{
		cBuffer.vMeshColor = BlackColor; // Siyah
	}

	XMMATRIX mTranslateSiyahSah = XMMatrixTranslation( sahSiyahKonum[0], sahSiyahKonum[1], sahSiyahKonum[2]);
	g_World = mTranslateSiyahSah;
	cBuffer.mWorld = XMMatrixTranspose( g_World );
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
	g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
	g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
	g_pImmediateContext->Draw(vertexCountSah, 0 );
	//-------------------------------------------------------------------------------------------------

	// Present our back buffer to our front buffer
	g_pSwapChain->Present( 0, 0 );

}

XMVECTOR testIntersection(XMVECTOR Ro, XMVECTOR Rd, SimpleVertex* verticesModel, int vertexCount, bool transform, XMMATRIX& g_World)
{
	for(int i = 0; i < vertexCount; i+=3)
	{
		XMVECTOR V0 = XMLoadFloat3(&verticesModel[i].Pos );
		XMVECTOR V1 = XMLoadFloat3(&verticesModel[i+1].Pos );
		XMVECTOR V2 = XMLoadFloat3(&verticesModel[i+2].Pos );

		if(transform)
		{
			V0 = XMVector3Transform(V0, g_World);
			V1 = XMVector3Transform(V1, g_World);
			V2 = XMVector3Transform(V2, g_World);
		}

		XMVECTOR Edge1 = V1 - V0;
		XMVECTOR Edge2 = V2 - V0;
		XMVECTOR Normal = XMVector3Cross(Edge1,Edge2);

		//Backface test
		XMVECTOR toEye = Eye - V0;
		if(XMVectorGetX(XMVector3Dot(Normal,toEye)) < 0.0F) continue;
		
		float D = -XMVectorGetX( XMVector3Dot(Normal,V0));

		float t = -(XMVectorGetX( XMVector3Dot(Normal,Ro)) + D) / XMVectorGetX( XMVector3Dot(Normal,Rd));

		if(t > 0.0F)
		{
			XMVECTOR R = Ro + t * Rd;

			XMVECTOR Edge3 = V2 - V1;
			XMVECTOR Edge4 = R - V0;
			XMVECTOR Edge5 = R - V1;

			float S  = XMVectorGetX(XMVector3Length(XMVector3Cross(Edge1,Edge2)));
			float s1 = XMVectorGetX(XMVector3Length(XMVector3Cross(Edge4,Edge2)));
			float s2 = XMVectorGetX(XMVector3Length(XMVector3Cross(Edge4,Edge1)));
			float s3 = XMVectorGetX(XMVector3Length(XMVector3Cross(Edge5,Edge3)));

			float fark = abs(S - (s1 + s2 + s3));

			float epsilon = 0.005F;
			 
			if (fark <= epsilon)
			{
				XMVECTOR Edge = V2 - V1;
				if (XMVectorGetX(XMVector3Length(Edge)) > 6.0F)
					return XMVECTOR((V1 + V2)/2);
				else
					return XMVECTOR((V0 + V1) / 2);
			}
		}		
	}	
	return XMVECTOR();
}

//hamlelerin tutuldugu matrisi resetler.
void RenderPointerHamleReset(){
	for(int i = 0; i<8;i++){
		for(int j = 0; j<8; j++){
			renderPointerHamle[i][j] = 0;
		}
	}
}

//hangi tasýn secýldýgýný belýrler
void SecilenTas(int *tas, bool *renk, int x, int z,bool *durum){
	if(oyunDurum[x][z][0] == 1){
		if(oyunDurum[x][z][1] == 1){*renk = true;*tas = 1;}
		else{*renk = false;*tas = 1;}
	}
	else if(oyunDurum[x][z][0] == 2){
		if(oyunDurum[x][z][1] == 1){*renk = true;*tas = 2;}
		else{*renk = false;*tas = 2;}		
	}
	else if(oyunDurum[x][z][0] == 3){
		if(oyunDurum[x][z][1] == 1){*renk = true;*tas = 3;}
		else{*renk = false;*tas = 3;}
	}
	else if(oyunDurum[x][z][0] == 4){
		if(oyunDurum[x][z][1] == 1){*renk = true;*tas = 4;}
		else{*renk = false;*tas = 4;}		
	}
	else if(oyunDurum[x][z][0] == 5){
		if(oyunDurum[x][z][1] == 1){*renk = true;*tas = 5;}
		else{*renk = false;*tas = 5;}		
	}
	else if(oyunDurum[x][z][0] == 6){
		if(oyunDurum[x][z][1] == 1){*renk = true;*tas = 6;}
		else{*renk = false;*tas = 6;}		
	}
	else
		durum = false;
}

//Secilen tasa ait hamleleri gösterir
void HamleGoster(int tas, bool renk, int x, int z, bool sahKontrol){
	//baslamadan once gecmiþ hamleleri resetle
	int gelenRenk;
	if (renk)
		gelenRenk = 2;
	else
		gelenRenk = 1;

	//sah tehlýke kontrolu yapýlýyorsa renderPointerHamle nýn kopyasý alýndý daha sonra ýlk halýne getýrýlmesý ýcýn
	int renderPointer[8][8];
	if(sahKontrol){
		for(int i = 0; i<8; i++){
			for(int j = 0; j<8; j++){
				renderPointer[i][j] = renderPointerHamle[i][j];
			}
		}
		RenderPointerHamleReset();
	}
	//piyon hamlesi
	if(tas == 1){
		if(renk){
			if(z == 1 && oyunDurum[x][z+1][1] == 0 && oyunDurum[x][z+2][1] == 0){
				renderPointerHamle[x][z+1] = 1;
				renderPointerHamle[x][z+2] = 1;
			}
			else if(z < 7 && oyunDurum[x][z+1][1] == 0){
				renderPointerHamle[x][z+1] = 1;
			}		
			//beyaz taslar icin yenecek tas varsa onlarda hamle olarak gosterilsin
			if(oyunDurum[x-1][z+1][1] == 2)
				renderPointerHamle[x-1][z+1] = 2;
			if(oyunDurum[x+1][z+1][1] == 2)
				renderPointerHamle[x+1][z+1] = 2;

			//Piyon icin ozel yandaký piyonu yeme hamlesý varmý
			if(z == 4){
				if(oyunDurum[x-1][z][0] == 1 && oyunDurum[x-1][z][1] == 2 && piyon2Hamle[0] == x-1 && piyon2Hamle[1] == z){
					renderPointerHamle[x-1][z+1] = 2;
				}
				if(oyunDurum[x+1][z][0] == 1 && oyunDurum[x+1][z][1] == 2 && piyon2Hamle[0] == x+1 && piyon2Hamle[1] == z){
					renderPointerHamle[x+1][z+1] = 2;
				}
			}
			//--------------------------------------------------
		}
		else{
			if(z == 6 && oyunDurum[x][z-1][1] == 0 && oyunDurum[x][z-2][1] == 0){
				renderPointerHamle[x][z-1] = 1;
				renderPointerHamle[x][z-2] = 1;
			}
			else if(z > 0 && oyunDurum[x][z-1][1] == 0){
				renderPointerHamle[x][z-1] = 1;
			}
			//siyah taslar icin yenecek taslar varsa hamle olarak gosterýlsýn
			if(oyunDurum[x-1][z-1][1] == 1)
				renderPointerHamle[x-1][z-1] = 2;
			if(oyunDurum[x+1][z-1][1] == 1)
				renderPointerHamle[x+1][z-1] = 2;

			//Piyon icin ozel yandaký piyonu yeme hamlesý varmý
			if(z == 3){
				if(oyunDurum[x-1][z][0] == 1 && oyunDurum[x-1][z][1] == 1 && piyon2Hamle[0] == x-1 && piyon2Hamle[1] == z){
					renderPointerHamle[x-1][z-1] = 2;
				}
				if(oyunDurum[x+1][z][0] == 1 && oyunDurum[x+1][z][1] == 1 && piyon2Hamle[0] == x+1 && piyon2Hamle[1] == z){
					renderPointerHamle[x+1][z-1] = 2;
				}
			}
			//--------------------------------------------------
		}
	}
	//at hamlesi
	else if(tas == 2){

		if(x >= 2){
			if(z > 0 && (oyunDurum[x-2][z-1][1] == 0 || oyunDurum[x-2][z-1][1] == gelenRenk)){
				if(oyunDurum[x-2][z-1][1] == gelenRenk)
					renderPointerHamle[x-2][z-1] = 2;
				else
					renderPointerHamle[x-2][z-1] = 1;
			}
			if(z < 7 && (oyunDurum[x-2][z+1][1] == 0 || oyunDurum[x-2][z+1][1] == gelenRenk)){
				if(oyunDurum[x-2][z+1][1] == gelenRenk)
					renderPointerHamle[x-2][z+1] = 2;
				else
					renderPointerHamle[x-2][z+1] = 1;
			}
		}
		if(x <= 5){
			if(z > 0 && (oyunDurum[x+2][z-1][1] == 0 || oyunDurum[x+2][z-1][1] == gelenRenk)){
				if(oyunDurum[x+2][z-1][1] == gelenRenk)
					renderPointerHamle[x+2][z-1] = 2;
				else
					renderPointerHamle[x+2][z-1] = 1;
			}
			if(z < 7 && (oyunDurum[x+2][z+1][1] == 0 || oyunDurum[x+2][z+1][1] == gelenRenk)){
				if(oyunDurum[x+2][z+1][1] == gelenRenk)
					renderPointerHamle[x+2][z+1] = 2;
				else
					renderPointerHamle[x+2][z+1] = 1;
			}
		}
		if(z >= 2){
			if(x > 0 && (oyunDurum[x-1][z-2][1] == 0 || oyunDurum[x-1][z-2][1] == gelenRenk)){
				if(oyunDurum[x-1][z-2][1] == gelenRenk)
					renderPointerHamle[x-1][z-2] = 2;
				else
					renderPointerHamle[x-1][z-2] = 1;
			}
			if(x < 7 && (oyunDurum[x+1][z-2][1] == 0 ||oyunDurum[x+1][z-2][1] == gelenRenk)){
				if(oyunDurum[x+1][z-2][1] == gelenRenk)
					renderPointerHamle[x+1][z-2] = 2;
				else
					renderPointerHamle[x+1][z-2] = 1;
			}
		}
		if(z <= 5){
			if(x > 0 && (oyunDurum[x-1][z+2][1] == 0 || oyunDurum[x-1][z+2][1] == gelenRenk)){
				if(oyunDurum[x-1][z+2][1] == gelenRenk)
					renderPointerHamle[x-1][z+2] = 2;
				else
					renderPointerHamle[x-1][z+2] = 1;
			}
			if(x < 7 && (oyunDurum[x+1][z+2][1] == 0 || oyunDurum[x+1][z+2][1] == gelenRenk)){
				if(oyunDurum[x+1][z+2][1] == gelenRenk)
					renderPointerHamle[x+1][z+2] = 2;
				else
					renderPointerHamle[x+1][z+2] = 1;
			}
		}
	}
	//fil hamlesi
	if(tas == 3 || tas == 5){
		int X = x;
		int Z = z;
		int kucuk,buyuk;
		if (x > z){
			kucuk = z;
			buyuk = x;
		}
		else{
			kucuk = x;
			buyuk = z;
		}

		for(int i = kucuk; i<8; i++){
			if(X < 7 && Z < 7){
				X++;
				Z++;
				if(oyunDurum[X][Z][0] != 0){
					if(oyunDurum[X][Z][1] == gelenRenk){
						renderPointerHamle[X][Z] = 2;
					}
					break;
				}
				else
					renderPointerHamle[X][Z] = 1;
			}
		}
		X = x;
		Z = z;
		for(int i = kucuk; i<8; i++){
			if(X < 7 && Z > 0){
				X++;
				Z--;
				if(oyunDurum[X][Z][0] != 0){
					if(oyunDurum[X][Z][1] == gelenRenk){
						renderPointerHamle[X][Z] = 2;
					}
					break;
				}
				else
					renderPointerHamle[X][Z] = 1;
			}
		}
		X = x;
		Z = z;
		for(int i = buyuk; i>0; i--){
			if(X > 0 && Z < 7){
				X--;
				Z++;
				if(oyunDurum[X][Z][0] != 0){
					if(oyunDurum[X][Z][1] == gelenRenk){
						renderPointerHamle[X][Z] = 2;
					}
					break;
				}
				else
					renderPointerHamle[X][Z] = 1;
			}
		}
		X = x;
		Z = z;
		for(int i = buyuk; i>0; i--){
			if(X > 0 && Z > 0){
				X--;
				Z--;
				if(oyunDurum[X][Z][0] != 0){
					if(oyunDurum[X][Z][1] == gelenRenk){
						renderPointerHamle[X][Z] = 2;
					}
					break;
				}
				else
					renderPointerHamle[X][Z] = 1;
			}
		}
	}
	//kale hamlesi
	if(tas == 4 || tas == 5){
		
		for(int i = z ; i<7; i++){
			if(oyunDurum[x][i+1][0] != 0){
				if(oyunDurum[x][i+1][1] == gelenRenk){
					renderPointerHamle[x][i+1] = 2;
				}
				break;
			}
			else{
				renderPointerHamle[x][i+1] = 1;
			}
		}

		for(int i = z ; i>0; i--){
			if(oyunDurum[x][i-1][0] != 0){
				if(oyunDurum[x][i-1][1] == gelenRenk){
					renderPointerHamle[x][i-1] = 2;
				}
				break;
			}
			else{
				renderPointerHamle[x][i-1] = 1;
			}
		}

		for(int i = x ; i<7; i++){
			if(oyunDurum[i+1][z][0] != 0){
				if(oyunDurum[i+1][z][1] == gelenRenk){
					renderPointerHamle[i+1][z] = 2;
				}
				break;
			}
			else{
				renderPointerHamle[i+1][z] = 1;
			}
		}

		for(int i = x ; i>0; i--){
			if(oyunDurum[i-1][z][0] != 0){
				if(oyunDurum[i-1][z][1] == gelenRenk){
					renderPointerHamle[i-1][z] = 2;
				}
				break;
			}
			else{
				renderPointerHamle[i-1][z] = 1;
			}
		}
	}
	//vezir hamlesi
	else if(tas == 5){
		//Vezir kale ile fil'in ozelliklerinin birleþimi olduðundan yukarda ýkýsý kullanýlarak yapýldý
	}
	//Þah hamlesi
	else if(tas == 6){
		if(x > 0){
			if(oyunDurum[x-1][z][0] != 0){
				if(oyunDurum[x-1][z][1] == gelenRenk)
					renderPointerHamle[x-1][z] = 2;
			}
			else
				renderPointerHamle[x-1][z] = 1;
		}
		if(x < 7){
			if(oyunDurum[x+1][z][0] != 0){
				if(oyunDurum[x+1][z][1] == gelenRenk)
					renderPointerHamle[x+1][z] = 2;
			}
			else
				renderPointerHamle[x+1][z] = 1;
		}
		if(z > 0){
			if(oyunDurum[x][z-1][0] != 0){
				if(oyunDurum[x][z-1][1] == gelenRenk)
					renderPointerHamle[x][z-1] = 2;
			}
			else
				renderPointerHamle[x][z-1] = 1;
		}
		if(z < 7){
			if(oyunDurum[x][z+1][0] != 0){
				if(oyunDurum[x][z+1][1] == gelenRenk)
					renderPointerHamle[x][z+1] = 2;
			}
			else
				renderPointerHamle[x][z+1] = 1;
		}
		if(x < 7 && z < 7){
			if(oyunDurum[x+1][z+1][0] != 0){
				if(oyunDurum[x+1][z+1][1] == gelenRenk)
					renderPointerHamle[x+1][z+1] = 2;
			}
			else
				renderPointerHamle[x+1][z+1] = 1;
		}
		if(x < 7 && z > 0){
			if(oyunDurum[x+1][z-1][0] != 0){
				if(oyunDurum[x+1][z-1][1] == gelenRenk)
					renderPointerHamle[x+1][z-1] = 2;
			}
			else
				renderPointerHamle[x+1][z-1] = 1;
		}
		if(x > 0 && z < 7){
			if(oyunDurum[x-1][z+1][0] != 0){
				if(oyunDurum[x-1][z+1][1] == gelenRenk)
					renderPointerHamle[x-1][z+1] = 2;
			}
			else
				renderPointerHamle[x-1][z+1] = 1;
		}
		if(x > 0 && z > 0){
			if(oyunDurum[x-1][z-1][0] != 0){
				if(oyunDurum[x-1][z-1][1] == gelenRenk)
					renderPointerHamle[x-1][z-1] = 2;
			}
			else
				renderPointerHamle[x-1][z-1] = 1;
		}
	}
	if(tas==1 || tas==2 || tas==3 || tas==4 || tas==5 || tas==6){
		renderPointerHamle[x][z] = 3;
	}

	//oyunDurum matrisi uzerýnde degýsýklýk yapýp kontrol edýldýgýnden burada oyunDurum matrýsýnden sah konumlarý elde edýldý.
	int sahBeyazX;
	int sahBeyazZ;
	int sahSiyahX;
	int sahSiyahZ;

	for(int i = 0; i<8; i++){
		for(int j = 0; j<8; j++){
			if(oyunDurum[i][j][0] == 6){
				if(oyunDurum[i][j][1] == 1){
					sahBeyazX = i;
					sahBeyazZ = j;
				}
				else if(oyunDurum[i][j][1] == 2){
					sahSiyahX = i;
					sahSiyahZ = j;					
				}
			}
		}
	}

	if(renderPointerHamle[sahBeyazX][sahBeyazZ] == 2){
		sahTehlikeDurum[0] = true;
	}
	if(renderPointerHamle[sahSiyahX][sahSiyahZ] == 2){
		sahTehlikeDurum[1] = true;
	}
	//////////////////////////////////////////////////////////
	if(sahKontrol){
		for(int i = 0; i<8; i++){
			for(int j = 0; j<8; j++){
				renderPointerHamle[i][j] = renderPointer[i][j];
			}
		}
	}
}

//Secilen hamleyi gerceklestirir
void HamleYap(int secimTas, bool secimRenk, int secimX, int secimZ, int yeniX, int yeniZ){
	bool piyonOzelDurum = true;
	//Piyon ozel capraz yeme hamlesi varmý
	if(secimTas == 1){
		if(secimRenk){
			if(oyunDurum[yeniX][yeniZ][0] == 0 && renderPointerHamle[yeniX][yeniZ] == 2){
				YenenTas(yeniX,yeniZ-1,!secimRenk);
				oyunDurum[yeniX][yeniZ-1][0] = 0;
				oyunDurum[yeniX][yeniZ-1][1] = 0;
				piyonOzelDurum = false;
			}
		}
		else{
			if(oyunDurum[yeniX][yeniZ][0] == 0 && renderPointerHamle[yeniX][yeniZ] == 2){
				YenenTas(yeniX,yeniZ+1,!secimRenk);
				oyunDurum[yeniX][yeniZ+1][0] = 0;
				oyunDurum[yeniX][yeniZ+1][1] = 0;
				piyonOzelDurum = false;
			}
		}
	}
	//-------------------------------------

	oyunDurum[yeniX][yeniZ][0] = oyunDurum[secimX][secimZ][0];
	oyunDurum[yeniX][yeniZ][1] = oyunDurum[secimX][secimZ][1];
	oyunDurum[secimX][secimZ][0] = 0;
	oyunDurum[secimX][secimZ][1] = 0;

	//Piyon ozel hamlesý baska hamleler olabýlme durumundan dolayý en son piyon2Hamle durumu sýlýnýr.
	piyon2Hamle[0] = 0;
	piyon2Hamle[1] = 0;
	//-------------------

	if(secimTas == 1 || secimTas == 5){
		if(secimRenk){
			for(int i = 0; i<8; i++){
				if(piyonBeyazKonum[i][0] == satrancTahtasiX[secimX] && piyonBeyazKonum[i][2] == satrancTahtasiZ[secimZ]){
					piyonBeyazKonumHareket[i][0] = satrancTahtasiX[yeniX];
					piyonBeyazKonumHareket[i][2] = satrancTahtasiZ[yeniZ];
					//Piyon ýlk olarak 2 hamle yaptýmý
					if(secimZ == 1 && yeniZ == 3){
						 piyon2Hamle[0] = yeniX;
						 piyon2Hamle[1] = yeniZ;
					}
					//--------------------------------
					break;

				}
			}
			//Piyon Vezir oldu
			if(yeniZ == 7){
				oyunDurum[yeniX][yeniZ][0] = 5;
			}
		}
		else{
			for(int i = 0; i<8; i++){
				if(piyonSiyahKonum[i][0] == satrancTahtasiX[secimX] && piyonSiyahKonum[i][2] == satrancTahtasiZ[secimZ]){
					piyonSiyahKonumHareket[i][0] = satrancTahtasiX[yeniX];
					piyonSiyahKonumHareket[i][2] = satrancTahtasiZ[yeniZ];
					//Piyon ýlk olarak 2 hamle yaptýmý
					if(secimZ == 6 && yeniZ == 4){
						 piyon2Hamle[0] = yeniX;
						 piyon2Hamle[1] = yeniZ;
					}
					//--------------------------------
					break;
				}
			}
			//Piyon Vezir oldu
			if(yeniZ == 0){
				oyunDurum[yeniX][yeniZ][0] = 5;
			}
		}
	}

	if(secimTas == 2){
		if(secimRenk){
			for(int i = 0; i<2; i++){
				if(atBeyazKonum[i][0] == satrancTahtasiX[secimX] && atBeyazKonum[i][2] == satrancTahtasiZ[secimZ]){
					atBeyazKonumHareket[i][0] = satrancTahtasiX[yeniX];
					atBeyazKonumHareket[i][2] = satrancTahtasiZ[yeniZ];
					break;
				}
			}
		}
		else{
			for(int i = 0; i<2; i++){
				if(atSiyahKonum[i][0] == satrancTahtasiX[secimX] && atSiyahKonum[i][2] == satrancTahtasiZ[secimZ]){
					atSiyahKonumHareket[i][0] = satrancTahtasiX[yeniX];
					atSiyahKonumHareket[i][2] = satrancTahtasiZ[yeniZ];
					break;
				}
			}
		}
	}

	if(secimTas == 3){
		if(secimRenk){
			for(int i = 0; i<2; i++){
				if(filBeyazKonum[i][0] == satrancTahtasiX[secimX] && filBeyazKonum[i][2] == satrancTahtasiZ[secimZ]){
					filBeyazKonumHareket[i][0] = satrancTahtasiX[yeniX];
					filBeyazKonumHareket[i][2] = satrancTahtasiZ[yeniZ];
					break;
				}
			}
		}
		else{
			for(int i = 0; i<2; i++){
				if(filSiyahKonum[i][0] == satrancTahtasiX[secimX] && filSiyahKonum[i][2] == satrancTahtasiZ[secimZ]){
					filSiyahKonumHareket[i][0] = satrancTahtasiX[yeniX];
					filSiyahKonumHareket[i][2] = satrancTahtasiZ[yeniZ];
					break;
				}
			}
		}
	}

	if(secimTas == 4){
		if(secimRenk){
			for(int i = 0; i<2; i++){
				if(kaleBeyazKonum[i][0] == satrancTahtasiX[secimX] && kaleBeyazKonum[i][2] == satrancTahtasiZ[secimZ]){
					kaleBeyazKonumHareket[i][0] = satrancTahtasiX[yeniX];
					kaleBeyazKonumHareket[i][2] = satrancTahtasiZ[yeniZ];
					break;
				}
			}
		}
		else{
			for(int i = 0; i<2; i++){
				if(kaleSiyahKonum[i][0] == satrancTahtasiX[secimX] && kaleSiyahKonum[i][2] == satrancTahtasiZ[secimZ]){
					kaleSiyahKonumHareket[i][0] = satrancTahtasiX[yeniX];
					kaleSiyahKonumHareket[i][2] = satrancTahtasiZ[yeniZ];
					break;
				}
			}
		}
	}

	if(secimTas == 5){
		if(secimRenk){
			if(vezirBeyazKonum[0] == satrancTahtasiX[secimX] && vezirBeyazKonum[2] == satrancTahtasiZ[secimZ]){
				vezirBeyazKonumHareket[0] = satrancTahtasiX[yeniX];
				vezirBeyazKonumHareket[2] = satrancTahtasiZ[yeniZ];
			}
		}
		else{
			if(vezirSiyahKonum[0] == satrancTahtasiX[secimX] && vezirSiyahKonum[2] == satrancTahtasiZ[secimZ]){
				vezirSiyahKonumHareket[0] = satrancTahtasiX[yeniX];
				vezirSiyahKonumHareket[2] = satrancTahtasiZ[yeniZ];
			}
		}
	}


	if(secimTas == 6){
		if(secimRenk){
			if(sahBeyazKonum[0] == satrancTahtasiX[secimX] && sahBeyazKonum[2] == satrancTahtasiZ[secimZ]){
				sahBeyazKonumHareket[0] = satrancTahtasiX[yeniX];
				sahBeyazKonumHareket[2] = satrancTahtasiZ[yeniZ];
			}
		}
		else{
			if(sahSiyahKonum[0] == satrancTahtasiX[secimX] && sahSiyahKonum[2] == satrancTahtasiZ[secimZ]){
				sahSiyahKonumHareket[0] = satrancTahtasiX[yeniX];
				sahSiyahKonumHareket[2] = satrancTahtasiZ[yeniZ];
			}
		}
	}
	//Yenen tasý bul
	if(renderPointerHamle[yeniX][yeniZ] == 2 && piyonOzelDurum){
		YenenTas(yeniX,yeniZ,!secimRenk);
	}
	else{
		//TasHareket sesi
		PlaySound(TEXT("tasHareket.wav"), NULL, SND_FILENAME | SND_ASYNC);
		////////////////////////////
	}
}

//Yenen tasý bulur
void YenenTas(int X , int Z , bool renk){
	//yenen tas sesi
	PlaySound(TEXT("yenenTas.wav"), NULL, SND_FILENAME | SND_ASYNC);
	//-----------------------

	if(renk){
		for(int i = 0; i<8; i++){
			if(piyonBeyazKonum[i][0] == satrancTahtasiX[X] && piyonBeyazKonum[i][2] == satrancTahtasiZ[Z]){
				piyonBeyazKonumHareket[i][0] = satrancTahtasiX[yenenBeyazX];
				piyonBeyazKonumHareket[i][2] = satrancTahtasiZ[yenenBeyazZ];
				yenenBeyazX++;
				break;
			}
		}
	}
	else{
		for(int i = 0; i<8; i++){
			if(piyonSiyahKonum[i][0] == satrancTahtasiX[X] && piyonSiyahKonum[i][2] == satrancTahtasiZ[Z]){
				piyonSiyahKonumHareket[i][0] = satrancTahtasiX[yenenSiyahX];
				piyonSiyahKonumHareket[i][2] = satrancTahtasiZ[yenenSiyahZ];
				yenenSiyahX++;
				break;
			}
		}
	}

	if(renk){
		for(int i = 0; i<2; i++){
			if(atBeyazKonum[i][0] == satrancTahtasiX[X] && atBeyazKonum[i][2] == satrancTahtasiZ[Z]){
				atBeyazKonumHareket[i][0] = satrancTahtasiX[yenenBeyazX];
				atBeyazKonumHareket[i][2] = satrancTahtasiZ[yenenBeyazZ];
				yenenBeyazX++;
				break;
			}
		}
	}
	else{
		for(int i = 0; i<2; i++){
			if(atSiyahKonum[i][0] == satrancTahtasiX[X] && atSiyahKonum[i][2] == satrancTahtasiZ[Z]){
				atSiyahKonumHareket[i][0] = satrancTahtasiX[yenenSiyahX];
				atSiyahKonumHareket[i][2] = satrancTahtasiZ[yenenSiyahZ];
				yenenSiyahX++;
				break;
			}
		}
	}

	if(renk){
		for(int i = 0; i<2; i++){
			if(filBeyazKonum[i][0] == satrancTahtasiX[X] && filBeyazKonum[i][2] == satrancTahtasiZ[Z]){
				filBeyazKonumHareket[i][0] = satrancTahtasiX[yenenBeyazX];
				filBeyazKonumHareket[i][2] = satrancTahtasiZ[yenenBeyazZ];
				yenenBeyazX++;
				break;
			}
		}
	}
	else{
		for(int i = 0; i<2; i++){
			if(filSiyahKonum[i][0] == satrancTahtasiX[X] && filSiyahKonum[i][2] == satrancTahtasiZ[Z]){
				filSiyahKonumHareket[i][0] = satrancTahtasiX[yenenSiyahX];
				filSiyahKonumHareket[i][2] = satrancTahtasiZ[yenenSiyahZ];
				yenenSiyahX++;
				break;
			}
		}
	}

	if(renk){
		for(int i = 0; i<2; i++){
			if(kaleBeyazKonum[i][0] == satrancTahtasiX[X] && kaleBeyazKonum[i][2] == satrancTahtasiZ[Z]){
				kaleBeyazKonumHareket[i][0] = satrancTahtasiX[yenenBeyazX];
				kaleBeyazKonumHareket[i][2] = satrancTahtasiZ[yenenBeyazZ];
				yenenBeyazX++;
				break;
			}
		}
	}
	else{
		for(int i = 0; i<2; i++){
			if(kaleSiyahKonum[i][0] == satrancTahtasiX[X] && kaleSiyahKonum[i][2] == satrancTahtasiZ[Z]){
				kaleSiyahKonumHareket[i][0] = satrancTahtasiX[yenenSiyahX];
				kaleSiyahKonumHareket[i][2] = satrancTahtasiZ[yenenSiyahZ];
				yenenSiyahX++;
				break;
			}
		}
	}

	if(renk){
		if(vezirBeyazKonum[0] == satrancTahtasiX[X] && vezirBeyazKonum[2] == satrancTahtasiZ[Z]){
			vezirBeyazKonumHareket[0] = satrancTahtasiX[yenenBeyazX];
			vezirBeyazKonumHareket[2] = satrancTahtasiZ[yenenBeyazZ];
			yenenBeyazX++;
		}
	}
	else{
		if(vezirSiyahKonum[0] == satrancTahtasiX[X] && vezirSiyahKonum[2] == satrancTahtasiZ[Z]){
			vezirSiyahKonumHareket[0] = satrancTahtasiX[yenenSiyahX];
			vezirSiyahKonumHareket[2] = satrancTahtasiZ[yenenSiyahZ];
			yenenSiyahX++;
		}
	}

	if(renk){
		if(sahBeyazKonum[0] == satrancTahtasiX[X] && sahBeyazKonum[2] == satrancTahtasiZ[Z]){
			sahBeyazKonumHareket[0] = satrancTahtasiX[yenenBeyazX];
			sahBeyazKonumHareket[2] = satrancTahtasiZ[yenenBeyazZ];
			sahTehlikeDurum[0] = false;
			yenenBeyazX++;
		}
	}
	else{
		if(sahSiyahKonum[0] == satrancTahtasiX[X] && sahSiyahKonum[2] == satrancTahtasiZ[Z]){
			sahSiyahKonumHareket[0] = satrancTahtasiX[yenenSiyahX];
			sahSiyahKonumHareket[2] = satrancTahtasiZ[yenenSiyahZ];
			sahTehlikeDurum[1] = false;
			yenenSiyahX++;
		}
	}
	
	if(yenenBeyazX >= 16){
		yenenBeyazX = 8;
		yenenBeyazZ += 2;
	}
	if(yenenSiyahX >= 16){
		yenenSiyahX = 8;
		yenenSiyahZ += 2; 
	}
}

//Taslarýn hareketini gercekler
void TasHareket(){
	//Beyaz piyonlar ýcýn anýmasyon hareketý
	for (int i = 0; i<8; i++){
		if(piyonBeyazKonumHareket[i][0] != piyonBeyazKonum[i][0] || piyonBeyazKonumHareket[i][2] != piyonBeyazKonum[i][2]){
			hareketVar[0] = true;
			if(piyonBeyazY[i] && (piyonBeyazKonumHareket[i][2] > 20 || piyonBeyazKonumHareket[i][2] < -20)){
				piyonBeyazKonum[i][1] += 2*tasHareketHiz; 
				if(piyonBeyazKonum[i][1] >= tasYmax){
					piyonBeyazY[i] = false;
				}
			}
			else{
				piyonBeyazY[i] = false;
				if(piyonBeyazKonum[i][0] > piyonBeyazKonumHareket[i][0]){
					piyonBeyazKonum[i][0] -= tasHareketHiz;
					if(piyonBeyazKonum[i][0] <= piyonBeyazKonumHareket[i][0]){
						piyonBeyazKonum[i][0] = piyonBeyazKonumHareket[i][0];
					}
				}
				else if(piyonBeyazKonum[i][0] < piyonBeyazKonumHareket[i][0]){
					piyonBeyazKonum[i][0] += tasHareketHiz;
					if(piyonBeyazKonum[i][0] >= piyonBeyazKonumHareket[i][0]){
						piyonBeyazKonum[i][0] = piyonBeyazKonumHareket[i][0];
					}
				}
				//burada hareket varsa Y eksenýnde hareket ettýrýlecek
				if(piyonBeyazKonum[i][2] > piyonBeyazKonumHareket[i][2]){
					piyonBeyazKonum[i][2] -= tasHareketHiz;
					if(piyonBeyazKonum[i][2] <= piyonBeyazKonumHareket[i][2]){
						piyonBeyazKonum[i][2] = piyonBeyazKonumHareket[i][2];
					}
				}
				else if(piyonBeyazKonum[i][2] < piyonBeyazKonumHareket[i][2]){
					piyonBeyazKonum[i][2] += tasHareketHiz;
					if(piyonBeyazKonum[i][2] >= piyonBeyazKonumHareket[i][2]){
						piyonBeyazKonum[i][2] = piyonBeyazKonumHareket[i][2];
					}
				}
			}
		}
		else if(!piyonBeyazY[i] && piyonBeyazKonum[i][1] >= 0 && piyonBeyazKonumHareket[i][0] == piyonBeyazKonum[i][0] && piyonBeyazKonumHareket[i][2] == piyonBeyazKonum[i][2] ){
			piyonBeyazKonum[i][1] -= tasHareketHiz;
			if(piyonBeyazKonum[i][1] < 0){
				piyonBeyazKonum[i][1] =0;
				piyonBeyazY[i] = true;
				hareketVar[0] = false;
			}
		}
		
		//Siyah piyonlar icin anýmasyon hareketý
		if(piyonSiyahKonumHareket[i][0] != piyonSiyahKonum[i][0] || piyonSiyahKonumHareket[i][2] != piyonSiyahKonum[i][2]){
			hareketVar[1] = true;
			if(piyonSiyahY[i] && (piyonSiyahKonumHareket[i][2] > 20 || piyonSiyahKonumHareket[i][2] < -20)){
				piyonSiyahKonum[i][1] += 2*tasHareketHiz; 
				if(piyonSiyahKonum[i][1] >= tasYmax){
					piyonSiyahY[i] = false;
				}
			}
			else{
				piyonSiyahY[i] = false;
				if(piyonSiyahKonum[i][0] > piyonSiyahKonumHareket[i][0]){
					piyonSiyahKonum[i][0] -= tasHareketHiz;
					if(piyonSiyahKonum[i][0] <= piyonSiyahKonumHareket[i][0]){
						piyonSiyahKonum[i][0] = piyonSiyahKonumHareket[i][0];
					}
				}
				else if(piyonSiyahKonum[i][0] < piyonSiyahKonumHareket[i][0]){
					piyonSiyahKonum[i][0] += tasHareketHiz;
					if(piyonSiyahKonum[i][0] >= piyonSiyahKonumHareket[i][0]){
						piyonSiyahKonum[i][0] = piyonSiyahKonumHareket[i][0];
					}
				}
				//burada hareket varsa Y eksenýnde hareket ettýrýlecek
				if(piyonSiyahKonum[i][2] > piyonSiyahKonumHareket[i][2]){
					piyonSiyahKonum[i][2] -= tasHareketHiz;
					if(piyonSiyahKonum[i][2] <= piyonSiyahKonumHareket[i][2]){
						piyonSiyahKonum[i][2] = piyonSiyahKonumHareket[i][2];
					}
				}
				else if(piyonSiyahKonum[i][2] < piyonSiyahKonumHareket[i][2]){
					piyonSiyahKonum[i][2] += tasHareketHiz;
					if(piyonSiyahKonum[i][2] >= piyonSiyahKonumHareket[i][2]){
						piyonSiyahKonum[i][2] = piyonSiyahKonumHareket[i][2];
					}
				}
			}
		}
		else if(!piyonSiyahY[i] && piyonSiyahKonum[i][1] >= 0 && piyonSiyahKonumHareket[i][0] == piyonSiyahKonum[i][0] && piyonSiyahKonumHareket[i][2] == piyonSiyahKonum[i][2] ){
			piyonSiyahKonum[i][1] -= tasHareketHiz;
			if(piyonSiyahKonum[i][1] < 0){
				piyonSiyahKonum[i][1] =0;
				piyonSiyahY[i] = true;
				hareketVar[1] = false;
			}
		}
	}

	//Atlar icin
	for (int i = 0; i<2; i++){
		//beyaz atlar icin
		if(atBeyazKonumHareket[i][0] != atBeyazKonum[i][0] || atBeyazKonumHareket[i][2] != atBeyazKonum[i][2]){
			hareketVar[2] = true;
			if(atBeyazY[i]){
				atBeyazKonum[i][1] += 2*tasHareketHiz; 
				if(atBeyazKonum[i][1] >= tasYmax){
					atBeyazY[i] = false;
				}
			}
			else{
				if(atBeyazKonum[i][0] > atBeyazKonumHareket[i][0]){
					atBeyazKonum[i][0] -= tasHareketHiz;
					if(atBeyazKonum[i][0] <= atBeyazKonumHareket[i][0]){
						atBeyazKonum[i][0] = atBeyazKonumHareket[i][0];
					}
				}
				else if(atBeyazKonum[i][0] < atBeyazKonumHareket[i][0]){
					atBeyazKonum[i][0] += tasHareketHiz;
					if(atBeyazKonum[i][0] >= atBeyazKonumHareket[i][0]){
						atBeyazKonum[i][0] = atBeyazKonumHareket[i][0];
					}
				}
				//burada hareket varsa Y eksenýnde hareket ettýrýlecek
				if(atBeyazKonum[i][2] > atBeyazKonumHareket[i][2]){
					atBeyazKonum[i][2] -= tasHareketHiz;
					if(atBeyazKonum[i][2] <= atBeyazKonumHareket[i][2]){
						atBeyazKonum[i][2] = atBeyazKonumHareket[i][2];
					}
				}
				else if(atBeyazKonum[i][2] < atBeyazKonumHareket[i][2]){
					atBeyazKonum[i][2] += tasHareketHiz;
					if(atBeyazKonum[i][2] >= atBeyazKonumHareket[i][2]){
						atBeyazKonum[i][2] = atBeyazKonumHareket[i][2];
					}
				}
			}
		}
		else if(!atBeyazY[i] && atBeyazKonum[i][1] >= 0 && atBeyazKonumHareket[i][0] == atBeyazKonum[i][0] && atBeyazKonumHareket[i][2] == atBeyazKonum[i][2] ){
			atBeyazKonum[i][1] -= tasHareketHiz;
			if(atBeyazKonum[i][1] < 0){
				atBeyazKonum[i][1] =0;
				atBeyazY[i] = true;
				hareketVar[2] = false;
			}
		}
		
		//siyah atlar icin
		if(atSiyahKonumHareket[i][0] != atSiyahKonum[i][0] || atSiyahKonumHareket[i][2] != atSiyahKonum[i][2]){
			hareketVar[3] = true;
			if(atSiyahY[i]){
				atSiyahKonum[i][1] += 2*tasHareketHiz; 
				if(atSiyahKonum[i][1] >= tasYmax){
					atSiyahY[i] = false;
				}
			}
			else{
				if(atSiyahKonum[i][0] > atSiyahKonumHareket[i][0]){
					atSiyahKonum[i][0] -= tasHareketHiz;
					if(atSiyahKonum[i][0] <= atSiyahKonumHareket[i][0]){
						atSiyahKonum[i][0] = atSiyahKonumHareket[i][0];
					}
				}
				else if(atSiyahKonum[i][0] < atSiyahKonumHareket[i][0]){
					atSiyahKonum[i][0] += tasHareketHiz;
					if(atSiyahKonum[i][0] >= atSiyahKonumHareket[i][0]){
						atSiyahKonum[i][0] = atSiyahKonumHareket[i][0];
					}
				}
				//burada hareket varsa Y eksenýnde hareket ettýrýlecek
				if(atSiyahKonum[i][2] > atSiyahKonumHareket[i][2]){
					atSiyahKonum[i][2] -= tasHareketHiz;
					if(atSiyahKonum[i][2] <= atSiyahKonumHareket[i][2]){
						atSiyahKonum[i][2] = atSiyahKonumHareket[i][2];
					}
				}
				else if(atSiyahKonum[i][2] < atSiyahKonumHareket[i][2]){
					atSiyahKonum[i][2] += tasHareketHiz;
					if(atSiyahKonum[i][2] >= atSiyahKonumHareket[i][2]){
						atSiyahKonum[i][2] = atSiyahKonumHareket[i][2];
					}
				}
			}
		}
		else if(!atSiyahY[i] && atSiyahKonum[i][1] >= 0 && atSiyahKonumHareket[i][0] == atSiyahKonum[i][0] && atSiyahKonumHareket[i][2] == atSiyahKonum[i][2] ){
			atSiyahKonum[i][1] -= tasHareketHiz;
			if(atSiyahKonum[i][1] < 0){
				atSiyahKonum[i][1] =0;
				atSiyahY[i] = true;
				hareketVar[3] = false;
			}
		}
	}

	//Fil icin
	for (int i = 0; i<2; i++){
		//beyaz Filler icin
		if(filBeyazKonumHareket[i][0] != filBeyazKonum[i][0] || filBeyazKonumHareket[i][2] != filBeyazKonum[i][2]){
			hareketVar[4] = true;
			if(filBeyazY[i] && (filBeyazKonumHareket[i][2] > 20 || filBeyazKonumHareket[i][2] < -20)){
				filBeyazKonum[i][1] += 2*tasHareketHiz; 
				if(filBeyazKonum[i][1] >= tasYmax){
					filBeyazY[i] = false;
				}
			}
			else{
				filBeyazY[i] = false;
				if(filBeyazKonum[i][0] > filBeyazKonumHareket[i][0]){
					filBeyazKonum[i][0] -= tasHareketHiz;
					if(filBeyazKonum[i][0] <= filBeyazKonumHareket[i][0]){
						filBeyazKonum[i][0] = filBeyazKonumHareket[i][0];
					}
				}
				else if(filBeyazKonum[i][0] < filBeyazKonumHareket[i][0]){
					filBeyazKonum[i][0] += tasHareketHiz;
					if(filBeyazKonum[i][0] >= filBeyazKonumHareket[i][0]){
						filBeyazKonum[i][0] = filBeyazKonumHareket[i][0];
					}
				}
				//burada hareket varsa Y eksenýnde hareket ettýrýlecek
				if(filBeyazKonum[i][2] > filBeyazKonumHareket[i][2]){
					filBeyazKonum[i][2] -= tasHareketHiz;
					if(filBeyazKonum[i][2] <= filBeyazKonumHareket[i][2]){
						filBeyazKonum[i][2] = filBeyazKonumHareket[i][2];
					}
				}
				else if(filBeyazKonum[i][2] < filBeyazKonumHareket[i][2]){
					filBeyazKonum[i][2] += tasHareketHiz;
					if(filBeyazKonum[i][2] >= filBeyazKonumHareket[i][2]){
						filBeyazKonum[i][2] = filBeyazKonumHareket[i][2];
					}
				}
			}
		}
		else if(!filBeyazY[i] && filBeyazKonum[i][1] >= 0 && filBeyazKonumHareket[i][0] == filBeyazKonum[i][0] && filBeyazKonumHareket[i][2] == filBeyazKonum[i][2] ){
			filBeyazKonum[i][1] -= tasHareketHiz;
			if(filBeyazKonum[i][1] < 0){
				filBeyazKonum[i][1] =0;
				filBeyazY[i] = true;
				hareketVar[4] = false;
			}
		}
		
		//siyah Filler icin
		if(filSiyahKonumHareket[i][0] != filSiyahKonum[i][0] || filSiyahKonumHareket[i][2] != filSiyahKonum[i][2]){
			hareketVar[5] = true;
			if(filSiyahY[i] && (filSiyahKonumHareket[i][2] > 20 || filSiyahKonumHareket[i][2] < -20)){
				filSiyahKonum[i][1] += 2*tasHareketHiz; 
				if(filSiyahKonum[i][1] >= tasYmax){
					filSiyahY[i] = false;
				}
			}
			else{
				filSiyahY[i] = false;
				if(filSiyahKonum[i][0] > filSiyahKonumHareket[i][0]){
					filSiyahKonum[i][0] -= tasHareketHiz;
					if(filSiyahKonum[i][0] <= filSiyahKonumHareket[i][0]){
						filSiyahKonum[i][0] = filSiyahKonumHareket[i][0];
					}
				}
				else if(filSiyahKonum[i][0] < filSiyahKonumHareket[i][0]){
					filSiyahKonum[i][0] += tasHareketHiz;
					if(filSiyahKonum[i][0] >= filSiyahKonumHareket[i][0]){
						filSiyahKonum[i][0] = filSiyahKonumHareket[i][0];
					}
				}
				//burada hareket varsa Y eksenýnde hareket ettýrýlecek
				if(filSiyahKonum[i][2] > filSiyahKonumHareket[i][2]){
					filSiyahKonum[i][2] -= tasHareketHiz;
					if(filSiyahKonum[i][2] <= filSiyahKonumHareket[i][2]){
						filSiyahKonum[i][2] = filSiyahKonumHareket[i][2];
					}
				}
				else if(filSiyahKonum[i][2] < filSiyahKonumHareket[i][2]){
					filSiyahKonum[i][2] += tasHareketHiz;
					if(filSiyahKonum[i][2] >= filSiyahKonumHareket[i][2]){
						filSiyahKonum[i][2] = filSiyahKonumHareket[i][2];
					}
				}
			}
		}
		else if(!filSiyahY[i] && filSiyahKonum[i][1] >= 0 && filSiyahKonumHareket[i][0] == filSiyahKonum[i][0] && filSiyahKonumHareket[i][2] == filSiyahKonum[i][2] ){
			filSiyahKonum[i][1] -= tasHareketHiz;
			if(filSiyahKonum[i][1] < 0){
				filSiyahKonum[i][1] =0;
				filSiyahY[i] = true;
				hareketVar[5] = false;
			}
		}
	}

	//kale icin
	for (int i = 0; i<2; i++){
		//beyaz kaleler icin
		if(kaleBeyazKonumHareket[i][0] != kaleBeyazKonum[i][0] || kaleBeyazKonumHareket[i][2] != kaleBeyazKonum[i][2]){
			hareketVar[6] = true;
			if(kaleBeyazY[i] && (kaleBeyazKonumHareket[i][2] > 20 || kaleBeyazKonumHareket[i][2] < -20)){
				kaleBeyazKonum[i][1] += 2*tasHareketHiz; 
				if(kaleBeyazKonum[i][1] >= tasYmax){
					kaleBeyazY[i] = false;
				}
			}
			else{
				kaleBeyazY[i] = false;
				if(kaleBeyazKonum[i][0] > kaleBeyazKonumHareket[i][0]){
					kaleBeyazKonum[i][0] -= tasHareketHiz;
					if(kaleBeyazKonum[i][0] <= kaleBeyazKonumHareket[i][0]){
						kaleBeyazKonum[i][0] = kaleBeyazKonumHareket[i][0];
					}
				}
				else if(kaleBeyazKonum[i][0] < kaleBeyazKonumHareket[i][0]){
					kaleBeyazKonum[i][0] += tasHareketHiz;
					if(kaleBeyazKonum[i][0] >= kaleBeyazKonumHareket[i][0]){
						kaleBeyazKonum[i][0] = kaleBeyazKonumHareket[i][0];
					}
				}
				//burada hareket varsa Y eksenýnde hareket ettýrýlecek
				if(kaleBeyazKonum[i][2] > kaleBeyazKonumHareket[i][2]){
					kaleBeyazKonum[i][2] -= tasHareketHiz;
					if(kaleBeyazKonum[i][2] <= kaleBeyazKonumHareket[i][2]){
						kaleBeyazKonum[i][2] = kaleBeyazKonumHareket[i][2];
					}
				}
				else if(kaleBeyazKonum[i][2] < kaleBeyazKonumHareket[i][2]){
					kaleBeyazKonum[i][2] += tasHareketHiz;
					if(kaleBeyazKonum[i][2] >= kaleBeyazKonumHareket[i][2]){
						kaleBeyazKonum[i][2] = kaleBeyazKonumHareket[i][2];
					}
				}
			}
		}
		else if(!kaleBeyazY[i] && kaleBeyazKonum[i][1] >= 0 && kaleBeyazKonumHareket[i][0] == kaleBeyazKonum[i][0] && kaleBeyazKonumHareket[i][2] == kaleBeyazKonum[i][2] ){
			kaleBeyazKonum[i][1] -= tasHareketHiz;
			if(kaleBeyazKonum[i][1] < 0){
				kaleBeyazKonum[i][1] =0;
				kaleBeyazY[i] = true;
				hareketVar[6] = false;;
			}
		}
		
		//siyah kaleler icin
		if(kaleSiyahKonumHareket[i][0] != kaleSiyahKonum[i][0] || kaleSiyahKonumHareket[i][2] != kaleSiyahKonum[i][2]){
			hareketVar[7] = true;
			if(kaleSiyahY[i] && (kaleSiyahKonumHareket[i][2] > 20 || kaleSiyahKonumHareket[i][2] < -20)){
				kaleSiyahKonum[i][1] += 2*tasHareketHiz; 
				if(kaleSiyahKonum[i][1] >= tasYmax){
					kaleSiyahY[i] = false;
				}
			}
			else{
				kaleSiyahY[i] = false;
				if(kaleSiyahKonum[i][0] > kaleSiyahKonumHareket[i][0]){
					kaleSiyahKonum[i][0] -= tasHareketHiz;
					if(kaleSiyahKonum[i][0] <= kaleSiyahKonumHareket[i][0]){
						kaleSiyahKonum[i][0] = kaleSiyahKonumHareket[i][0];
					}
				}
				else if(kaleSiyahKonum[i][0] < kaleSiyahKonumHareket[i][0]){
					kaleSiyahKonum[i][0] += tasHareketHiz;
					if(kaleSiyahKonum[i][0] >= kaleSiyahKonumHareket[i][0]){
						kaleSiyahKonum[i][0] = kaleSiyahKonumHareket[i][0];
					}
				}
				//burada hareket varsa Y eksenýnde hareket ettýrýlecek
				if(kaleSiyahKonum[i][2] > kaleSiyahKonumHareket[i][2]){
					kaleSiyahKonum[i][2] -= tasHareketHiz;
					if(kaleSiyahKonum[i][2] <= kaleSiyahKonumHareket[i][2]){
						kaleSiyahKonum[i][2] = kaleSiyahKonumHareket[i][2];
					}
				}
				else if(kaleSiyahKonum[i][2] < kaleSiyahKonumHareket[i][2]){
					kaleSiyahKonum[i][2] += tasHareketHiz;
					if(kaleSiyahKonum[i][2] >= kaleSiyahKonumHareket[i][2]){
						kaleSiyahKonum[i][2] = kaleSiyahKonumHareket[i][2];
					}
				}
			}
		}
		else if(!kaleSiyahY[i] && kaleSiyahKonum[i][1] >= 0 && kaleSiyahKonumHareket[i][0] == kaleSiyahKonum[i][0] && kaleSiyahKonumHareket[i][2] == kaleSiyahKonum[i][2] ){
			kaleSiyahKonum[i][1] -= tasHareketHiz;
			if(kaleSiyahKonum[i][1] < 0){
				kaleSiyahKonum[i][1] =0;
				kaleSiyahY[i] = true;
				hareketVar[7] = false;
			}
		}
	}

	//beyaz vezir icin
	if(vezirBeyazKonumHareket[0] != vezirBeyazKonum[0] || vezirBeyazKonumHareket[2] != vezirBeyazKonum[2]){
		hareketVar[8] = true;
		if(vezirBeyazY && (vezirBeyazKonumHareket[2] > 20 || vezirBeyazKonumHareket[2] < -20)){
			vezirBeyazKonum[1] += 2*tasHareketHiz; 
			if(vezirBeyazKonum[1] >= tasYmax){
				vezirBeyazY = false;
			}
		}
		else{
			vezirBeyazY = false;
			if(vezirBeyazKonum[0] > vezirBeyazKonumHareket[0]){
				vezirBeyazKonum[0] -= tasHareketHiz;
				if(vezirBeyazKonum[0] <= vezirBeyazKonumHareket[0]){
					vezirBeyazKonum[0] = vezirBeyazKonumHareket[0];
				}
			}
			else if(vezirBeyazKonum[0] < vezirBeyazKonumHareket[0]){
				vezirBeyazKonum[0] += tasHareketHiz;
				if(vezirBeyazKonum[0] >= vezirBeyazKonumHareket[0]){
					vezirBeyazKonum[0] = vezirBeyazKonumHareket[0];
				}
			}
			//burada hareket varsa Y eksenýnde hareket ettýrýlecek
			if(vezirBeyazKonum[2] > vezirBeyazKonumHareket[2]){
				vezirBeyazKonum[2] -= tasHareketHiz;
				if(vezirBeyazKonum[2] <= vezirBeyazKonumHareket[2]){
					vezirBeyazKonum[2] = vezirBeyazKonumHareket[2];
				}
			}
			else if(vezirBeyazKonum[2] < vezirBeyazKonumHareket[2]){
				vezirBeyazKonum[2] += tasHareketHiz;
				if(vezirBeyazKonum[2] >= vezirBeyazKonumHareket[2]){
					vezirBeyazKonum[2] = vezirBeyazKonumHareket[2];
				}
			}
		}
	}
	else if(!vezirBeyazY && vezirBeyazKonum[1] >= 0 && vezirBeyazKonumHareket[0] == vezirBeyazKonum[0] && vezirBeyazKonumHareket[2] == vezirBeyazKonum[2] ){
		vezirBeyazKonum[1] -= tasHareketHiz;
		if(vezirBeyazKonum[1] < 0){
			vezirBeyazKonum[1] =0;
			vezirBeyazY = true;
			hareketVar[8] = false;
		}
	}

	//Siyah vezir icin
	if(vezirSiyahKonumHareket[0] != vezirSiyahKonum[0] || vezirSiyahKonumHareket[2] != vezirSiyahKonum[2]){
		hareketVar[9] = true;
		if(vezirSiyahY && (vezirSiyahKonumHareket[2] > 20 || vezirSiyahKonumHareket[2] < -20)){
			vezirSiyahKonum[1] += 2*tasHareketHiz; 
			if(vezirSiyahKonum[1] >= tasYmax){
				vezirSiyahY = false;
			}
		}
		else{
			vezirSiyahY = false;
			if(vezirSiyahKonum[0] > vezirSiyahKonumHareket[0]){
				vezirSiyahKonum[0] -= tasHareketHiz;
				if(vezirSiyahKonum[0] <= vezirSiyahKonumHareket[0]){
					vezirSiyahKonum[0] = vezirSiyahKonumHareket[0];
				}
			}
			else if(vezirSiyahKonum[0] < vezirSiyahKonumHareket[0]){
				vezirSiyahKonum[0] += tasHareketHiz;
				if(vezirSiyahKonum[0] >= vezirSiyahKonumHareket[0]){
					vezirSiyahKonum[0] = vezirSiyahKonumHareket[0];
				}
			}
			//burada hareket varsa Y eksenýnde hareket ettýrýlecek
			if(vezirSiyahKonum[2] > vezirSiyahKonumHareket[2]){
				vezirSiyahKonum[2] -= tasHareketHiz;
				if(vezirSiyahKonum[2] <= vezirSiyahKonumHareket[2]){
					vezirSiyahKonum[2] = vezirSiyahKonumHareket[2];
				}
			}
			else if(vezirSiyahKonum[2] < vezirSiyahKonumHareket[2]){
				vezirSiyahKonum[2] += tasHareketHiz;
				if(vezirSiyahKonum[2] >= vezirSiyahKonumHareket[2]){
					vezirSiyahKonum[2] = vezirSiyahKonumHareket[2];
				}
			}
		}
	}
	else if(!vezirSiyahY && vezirSiyahKonum[1] >= 0 && vezirSiyahKonumHareket[0] == vezirSiyahKonum[0] && vezirSiyahKonumHareket[2] == vezirSiyahKonum[2] ){
		vezirSiyahKonum[1] -= tasHareketHiz;
		if(vezirSiyahKonum[1] < 0){
			vezirSiyahKonum[1] =0;
			vezirSiyahY = true;
			hareketVar[9] = false;
		}
	}

	//beyaz sah icin
	if(sahBeyazKonumHareket[0] != sahBeyazKonum[0] || sahBeyazKonumHareket[2] != sahBeyazKonum[2]){
		hareketVar[10] = true;
		if(sahBeyazY && (sahBeyazKonumHareket[2] > 20 || sahBeyazKonumHareket[2] < -20)){
			sahBeyazKonum[1] += 2*tasHareketHiz; 
			if(sahBeyazKonum[1] >= tasYmax){
				sahBeyazY = false;
			}
		}
		else{
			sahBeyazY = false;
			if(sahBeyazKonum[0] > sahBeyazKonumHareket[0]){
				sahBeyazKonum[0] -= tasHareketHiz;
				if(sahBeyazKonum[0] <= sahBeyazKonumHareket[0]){
					sahBeyazKonum[0] = sahBeyazKonumHareket[0];
				}
			}
			else if(sahBeyazKonum[0] < sahBeyazKonumHareket[0]){
				sahBeyazKonum[0] += tasHareketHiz;
				if(sahBeyazKonum[0] >= sahBeyazKonumHareket[0]){
					sahBeyazKonum[0] = sahBeyazKonumHareket[0];
				}
			}
			//burada hareket varsa Y eksenýnde hareket ettýrýlecek
			if(sahBeyazKonum[2] > sahBeyazKonumHareket[2]){
				sahBeyazKonum[2] -= tasHareketHiz;
				if(sahBeyazKonum[2] <= sahBeyazKonumHareket[2]){
					sahBeyazKonum[2] = sahBeyazKonumHareket[2];
				}
			}
			else if(sahBeyazKonum[2] < sahBeyazKonumHareket[2]){
				sahBeyazKonum[2] += tasHareketHiz;
				if(sahBeyazKonum[2] >= sahBeyazKonumHareket[2]){
					sahBeyazKonum[2] = sahBeyazKonumHareket[2];
				}
			}
		}
	}
	else if(!sahBeyazY && sahBeyazKonum[1] >= 0 && sahBeyazKonumHareket[0] == sahBeyazKonum[0] && sahBeyazKonumHareket[2] == sahBeyazKonum[2] ){
		sahBeyazKonum[1] -= tasHareketHiz;
		if(sahBeyazKonum[1] < 0){
			sahBeyazKonum[1] =0;
			sahBeyazY = true;
			hareketVar[10] = false;
		}
	}

	//Siyah sah icin
	if(sahSiyahKonumHareket[0] != sahSiyahKonum[0] || sahSiyahKonumHareket[2] != sahSiyahKonum[2]){
		hareketVar[11] = true;
		if(sahSiyahY && (sahSiyahKonumHareket[2] > 20 || sahSiyahKonumHareket[2] < -20)){
			sahSiyahKonum[1] += 2*tasHareketHiz; 
			if(sahSiyahKonum[1] >= tasYmax){
				sahSiyahY = false;
			}
		}
		else{
			sahSiyahY = false;
			if(sahSiyahKonum[0] > sahSiyahKonumHareket[0]){
				sahSiyahKonum[0] -= tasHareketHiz;
				if(sahSiyahKonum[0] <= sahSiyahKonumHareket[0]){
					sahSiyahKonum[0] = sahSiyahKonumHareket[0];
				}
			}
			else if(sahSiyahKonum[0] < sahSiyahKonumHareket[0]){
				sahSiyahKonum[0] += tasHareketHiz;
				if(sahSiyahKonum[0] >= sahSiyahKonumHareket[0]){
					sahSiyahKonum[0] = sahSiyahKonumHareket[0];
				}
			}
			//burada hareket varsa Y eksenýnde hareket ettýrýlecek
			if(sahSiyahKonum[2] > sahSiyahKonumHareket[2]){
				sahSiyahKonum[2] -= tasHareketHiz;
				if(sahSiyahKonum[2] <= sahSiyahKonumHareket[2]){
					sahSiyahKonum[2] = sahSiyahKonumHareket[2];
				}
			}
			else if(sahSiyahKonum[2] < sahSiyahKonumHareket[2]){
				sahSiyahKonum[2] += tasHareketHiz;
				if(sahSiyahKonum[2] >= sahSiyahKonumHareket[2]){
					sahSiyahKonum[2] = sahSiyahKonumHareket[2];
				}
			}
		}
	}
	else if(!sahSiyahY && sahSiyahKonum[1] >= 0 && sahSiyahKonumHareket[0] == sahSiyahKonum[0] && sahSiyahKonumHareket[2] == sahSiyahKonum[2] ){
		sahSiyahKonum[1] -= tasHareketHiz;
		if(sahSiyahKonum[1] < 0){
			sahSiyahKonum[1] =0;
			sahSiyahY = true;
			hareketVar[11] = false;
		}
	}
	
	//Burada hamle býtýnceyekadar hamleler gosterilecek sonra kapatýlacak ve oyun sýrasý degisecek
	if(KontrolTasHareket() && hamleDurum){
		RenderPointerHamleReset();
		hamleDurum = false;
		oyunSirasi = !oyunSirasi;

		//Sah tehlike durumunda ise SAHMAT kontrolu yap yoksa PAT kontrolu yap.
		int indis;
		if(oyunSirasi)
			indis = 0;
		else
			indis = 1;
		if(sahTehlikeDurum[indis]){
			SahMat();
		}
		else{
			Pat();
		}
		/////////////////////////////////////////

		//Oyun Sýrasýna gore Bakýs noktasýný degiþtir
		if(oyunSirasi){
			Eye = XMVectorSet( 0.0f, 25.0f, -45.0f, 0.0f );
			camYaw = 0;
		}
		else{
			Eye = XMVectorSet( 0.0f, 25.0f, 45.0f, 0.0f );
			camYaw = 22;
		}
		camPitch = 0.5f;
		UpdateCamera();

		//Yený oyuncunun hamlesý olan taslarý goster
		HamleOlanTaslariGoster();
		//-----------------------------------------
	}
	//////////////////////////////////////////////////////////////

}

//Hareket halýnde bir tas varmý kontrol yapar
bool KontrolTasHareket(void){
	for(int i = 0; i<12; i++){
		if(hareketVar[i]){
			return false;
		}
	}
	return true;
}

//Sah tehlikedemi kontrol eder
void SahTeklikeKontrol(){

	// sah tehlike durumlarýný kosmadan tehlýkelerý kaldýrdýk. 
	sahTehlikeDurum[0] = false;
	sahTehlikeDurum[1] = false;

	//sah tehlýkesý varmý tum durumlar ýncelenýyor
	for(int i = 0; i<8; i++){
		for(int j = 0; j<8; j++){
			if(oyunDurum[i][j][0] != 0){
				bool tasRenk;
				if(oyunDurum[i][j][1] == 1)
					tasRenk = true;
				else
					tasRenk = false;

				HamleGoster(oyunDurum[i][j][0],tasRenk,i,j,true);
			}
		}
	}
}

//Hamlenin kontrolunu yapar yapýlabýlýr býr hamlemý degýlmý
void HamleKontol(int secimX,int secimZ,bool sahMat){
	int renk,indis;
	if(oyunSirasi){
		renk = 2;
		indis = 0;
	}
	else{
		renk = 1;
		indis = 1;
	}

	for(int i = 0; i<8; i++){
		for(int j = 0; j<8; j++){
			if(renderPointerHamle[i][j] == 1 || renderPointerHamle[i][j] == 2){
					int kopyaTas = oyunDurum[i][j][0];
					int kopyaRenk = oyunDurum[i][j][1];
					oyunDurum[i][j][0] = oyunDurum[secimX][secimZ][0];
					oyunDurum[i][j][1] = oyunDurum[secimX][secimZ][1];
					oyunDurum[secimX][secimZ][0] = 0;
					oyunDurum[secimX][secimZ][1] = 0;

					//Burada sah tehlike durumuna bakýlacak
					renderPointerHamleDurum = false;
					SahTeklikeKontrol();
					renderPointerHamleDurum = true;
					//////////////////////////////////////////
					
					oyunDurum[secimX][secimZ][0] = oyunDurum[i][j][0];
					oyunDurum[secimX][secimZ][1] = oyunDurum[i][j][1];
					oyunDurum[i][j][0] = kopyaTas;
					oyunDurum[i][j][1] = kopyaRenk;

					if(sahTehlikeDurum[indis]) // bu hamle sahTehlike durumunu bozmuyorsa hamleyý kapat
						renderPointerHamle[i][j] = 0;
			}
		}
	}
	//Tasýn hamlesý yoksa tasHamlesi yok sesi cýkar
	if(RenderHamleBosKontrol() && !sahMat){
		PlaySound(TEXT("hamleYok.wav"), NULL, SND_FILENAME | SND_ASYNC);
	}
}

//Herhangý býr hamle varmý yokmu
bool RenderHamleBosKontrol(void){
	for(int i = 0; i<8; i++){
		for(int j = 0; j<8; j++){
			if(renderPointerHamle[i][j] == 1 || renderPointerHamle[i][j] == 2)
				return false;
		}
	}
	return true;
}

//Mat oldumu kontrolu yapar
void SahMat(void){
	int renk;
	if(oyunSirasi)
		renk = 1;
	else
		renk = 2;

	for(int i = 0; i<8; i++){
		for(int j = 0; j<8; j++){
			if(oyunDurum[i][j][0] != 0 && oyunDurum[i][j][1] == renk){
				RenderPointerHamleReset();
				bool tasRenk;
				if(oyunDurum[i][j][1] == 1)
					tasRenk = true;
				else
					tasRenk = false;
				HamleGoster(oyunDurum[i][j][0],tasRenk,i,j,false);
				HamleKontol(i,j,true);
				if(!RenderHamleBosKontrol()){
					RenderPointerHamleReset();
					return;
				}
			}
		}
	}
	RenderPointerHamleReset();

	//MAT için ses
	PlaySound(TEXT("sahMat.wav"), NULL, SND_FILENAME | SND_ASYNC);
	////////////////////////////

	if(oyunSirasi){
		if(MessageBox(NULL, TEXT("Siyah Kazandý.\nYeni oyun baþlatýlsýnmý ?"),TEXT("OYUN SONUCU"), MB_YESNO | MB_ICONINFORMATION) == IDYES){
			OyunYenile();
		}
		else{
			PostMessage(g_hWnd, WM_DESTROY, 0, 0);//Oyunu Sonlandýr
		}
	}
	else{
		if(MessageBox(NULL, TEXT("Beyaz Kazandý.\nYeni oyun baþlatýlsýnmý ?"),TEXT("OYUN SONUCU"), MB_YESNO | MB_ICONINFORMATION) == IDYES){
			OyunYenile();
		}
		else{
			PostMessage(g_hWnd, WM_DESTROY, 0, 0);//Oyunu Sonlandýr
		}
	}
}

//Pat kontrolu yapar
void Pat(void){
	int renk;
	if(oyunSirasi)
		renk = 1;
	else
		renk = 2;

	for(int i = 0; i<8; i++){
		for(int j = 0; j<8; j++){
			if(oyunDurum[i][j][0] != 0 && oyunDurum[i][j][1] == renk){
				RenderPointerHamleReset();
				bool tasRenk;
				if(oyunDurum[i][j][1] == 1)
					tasRenk = true;
				else
					tasRenk = false;
				HamleGoster(oyunDurum[i][j][0],tasRenk,i,j,false);
				HamleKontol(i,j,true);
				if(!RenderHamleBosKontrol()){
					RenderPointerHamleReset();
					return;
				}
			}
		}
	}
	RenderPointerHamleReset();

	//PAT için ses
	PlaySound(TEXT("pat.wav"), NULL, SND_FILENAME | SND_ASYNC);
	////////////////////////////

	if(MessageBox(NULL, TEXT("Oyun Berabere Sonuçlandý.\nYeni oyun baþlatýlsýnmý ?"),TEXT("OYUN SONUCU"), MB_YESNO | MB_ICONINFORMATION) == IDYES){
		OyunYenile();
	}
	else{
		PostMessage(g_hWnd, WM_DESTROY, 0, 0);//Oyunu Sonlandýr
	}
}

//Hamle Yapýlabýlecek taslarý ýsaretle
void HamleOlanTaslariGoster(){
	int renk;
	if(oyunSirasi)
		renk = 1;
	else
		renk = 2;
	int geciciHamle[8][8] = {0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0};

	for(int i = 0; i<8; i++){
		for(int j = 0; j<8; j++){
			if(oyunDurum[i][j][0] != 0 && oyunDurum[i][j][1] == renk){
				RenderPointerHamleReset();
				bool tasRenk;
				if(oyunDurum[i][j][1] == 1)
					tasRenk = true;
				else
					tasRenk = false;
				HamleGoster(oyunDurum[i][j][0],tasRenk,i,j,false);
				HamleKontol(i,j,true);
				if(!RenderHamleBosKontrol()){
					geciciHamle[i][j] = 5; 
				}
			}
		}
	}

	//GeciciHamle matrisini renderPointerHamle ye ata
	for(int i = 0; i<8; i++){
		for(int j = 0; j<8; j++){
			renderPointerHamle[i][j] = geciciHamle[i][j];
		}
	}

	//Hamle olan taslar gosterýlýnce cýkan ses
	PlaySound(TEXT("hamleOlanTaslariGoster.wav"), NULL, SND_FILENAME | SND_ASYNC);
	////////////////////////////
}

//Oyunu yeniden Baslatmak icin atamalar yapýlýr
void OyunYenile(void){
hamleDurum = false;
oyunSirasi = true;
//Oyun Sýrasýna gore Bakýs noktasýný degiþtir
if(oyunSirasi){
	Eye = XMVectorSet( 0.0f, 25.0f, -45.0f, 0.0f );
	camYaw = 0;
}
else{
	Eye = XMVectorSet( 0.0f, 25.0f, 45.0f, 0.0f );
	camYaw = 22;
}
camPitch = 0.5f;
UpdateCamera();
//-----------------------------------------------

renderPointerHamleDurum = true;
sahTehlikeDurum[0] = false;
sahTehlikeDurum[1] = false;
sahTehlikeDurumRender[0] = false;
sahTehlikeDurumRender[1] = false;
piyon2Hamle[0] = 0;//X
piyon2Hamle[1] = 0;//Z

for(int i = 0; i<12; i++)
	hareketVar[i] = false;

for(int i = 0; i<8; i++){
	piyonBeyazY[i] = true;
	piyonSiyahY[i] = true;
}

for(int i = 0; i<2; i++){
	atBeyazY[i] = true;
	atSiyahY[i] = true;
	filBeyazY[i] = true;
	filSiyahY[i] = true;
	kaleBeyazY[i] = true;
	kaleSiyahY[i] = true;
}

vezirBeyazY = true;
vezirSiyahY = true;
sahBeyazY = true;
sahSiyahY = true;

//Gecici olarak oyun matrýsý olusturduk kolay atama yapmak ýcýn
int oyunDurumGecici[8][8][2] = {{{4,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{4,2}},
								{{2,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{2,2}},
								{{3,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{3,2}},
								{{5,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{5,2}},
								{{6,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{6,2}},
								{{3,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{3,2}},
								{{2,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{2,2}},
								{{4,1},{1,1},{0,0},{0,0},{0,0},{0,0},{1,2},{4,2}}};
for(int i = 0; i<8; i++){
	for(int j = 0; j<8; j++){
		oyunDurum[i][j][0] = oyunDurumGecici[i][j][0];
		oyunDurum[i][j][1] = oyunDurumGecici[i][j][1];
	}
}

//Gecici tas konumlarý olusturuldu kolay atama ýcýn
float piyonBeyazKonumGecici[8][3] = {{satrancTahtasiX[0],0,satrancTahtasiZ[1]},{satrancTahtasiX[1],0,satrancTahtasiZ[1]},{satrancTahtasiX[2],0,satrancTahtasiZ[1]},{satrancTahtasiX[3],0,satrancTahtasiZ[1]},{satrancTahtasiX[4],0,satrancTahtasiZ[1]},{satrancTahtasiX[5],0,satrancTahtasiZ[1]},{satrancTahtasiX[6],0,satrancTahtasiZ[1]},{satrancTahtasiX[7],0,satrancTahtasiZ[1]}};
float piyonSiyahKonumGecici[8][3] = {{satrancTahtasiX[0],0,satrancTahtasiZ[6]},{satrancTahtasiX[1],0,satrancTahtasiZ[6]},{satrancTahtasiX[2],0,satrancTahtasiZ[6]},{satrancTahtasiX[3],0,satrancTahtasiZ[6]},{satrancTahtasiX[4],0,satrancTahtasiZ[6]},{satrancTahtasiX[5],0,satrancTahtasiZ[6]},{satrancTahtasiX[6],0,satrancTahtasiZ[6]},{satrancTahtasiX[7],0,satrancTahtasiZ[6]}};
float atBeyazKonumGecici[2][3] = {{satrancTahtasiX[1],0,satrancTahtasiZ[0]},{satrancTahtasiX[6],0,satrancTahtasiZ[0]}};
float atSiyahKonumGecici[2][3] = {{satrancTahtasiX[1],0,satrancTahtasiZ[7]},{satrancTahtasiX[6],0,satrancTahtasiZ[7]}};
float filBeyazKonumGecici[2][3] = {{satrancTahtasiX[2],0,satrancTahtasiZ[0]},{satrancTahtasiX[5],0,satrancTahtasiZ[0]}};
float filSiyahKonumGecici[2][3] = {{satrancTahtasiX[2],0,satrancTahtasiZ[7]},{satrancTahtasiX[5],0,satrancTahtasiZ[7]}};
float kaleBeyazKonumGecici[2][3] = {{satrancTahtasiX[0],0,satrancTahtasiZ[0]},{satrancTahtasiX[7],0,satrancTahtasiZ[0]}};
float kaleSiyahKonumGecici[2][3] = {{satrancTahtasiX[0],0,satrancTahtasiZ[7]},{satrancTahtasiX[7],0,satrancTahtasiZ[7]}};
float vezirBeyazKonumGecici[3] = {satrancTahtasiX[3],0,satrancTahtasiZ[0]};
float vezirSiyahKonumGecici[3] = {satrancTahtasiX[3],0,satrancTahtasiZ[7]};
float sahBeyazKonumGecici[3] = {satrancTahtasiX[4],0,satrancTahtasiZ[0]};
float sahSiyahKonumGecici[3] = {satrancTahtasiX[4],0,satrancTahtasiZ[7]};

for(int i = 0; i<8; i++){
	piyonBeyazKonum[i][0] = piyonBeyazKonumHareket[i][0] = piyonBeyazKonumGecici[i][0];
	piyonBeyazKonum[i][1] = piyonBeyazKonumHareket[i][1] = piyonBeyazKonumGecici[i][1];
	piyonBeyazKonum[i][2] = piyonBeyazKonumHareket[i][2] = piyonBeyazKonumGecici[i][2];

	piyonSiyahKonum[i][0] = piyonSiyahKonumHareket[i][0] = piyonSiyahKonumGecici[i][0];
	piyonSiyahKonum[i][1] = piyonSiyahKonumHareket[i][1] = piyonSiyahKonumGecici[i][1];
	piyonSiyahKonum[i][2] = piyonSiyahKonumHareket[i][2] = piyonSiyahKonumGecici[i][2];
}

for(int i = 0; i<2; i++){
	atBeyazKonum[i][0] = atBeyazKonumHareket[i][0] = atBeyazKonumGecici[i][0];
	atBeyazKonum[i][1] = atBeyazKonumHareket[i][1] = atBeyazKonumGecici[i][1];
	atBeyazKonum[i][2] = atBeyazKonumHareket[i][2] = atBeyazKonumGecici[i][2];

	atSiyahKonum[i][0] = atSiyahKonumHareket[i][0] = atSiyahKonumGecici[i][0];
	atSiyahKonum[i][1] = atSiyahKonumHareket[i][1] = atSiyahKonumGecici[i][1];
	atSiyahKonum[i][2] = atSiyahKonumHareket[i][2] = atSiyahKonumGecici[i][2];

	filBeyazKonum[i][0] = filBeyazKonumHareket[i][0] = filBeyazKonumGecici[i][0];
	filBeyazKonum[i][1] = filBeyazKonumHareket[i][1] = filBeyazKonumGecici[i][1];
	filBeyazKonum[i][2] = filBeyazKonumHareket[i][2] = filBeyazKonumGecici[i][2];

	filSiyahKonum[i][0] = filSiyahKonumHareket[i][0] = filSiyahKonumGecici[i][0];
	filSiyahKonum[i][1] = filSiyahKonumHareket[i][1] = filSiyahKonumGecici[i][1];
	filSiyahKonum[i][2] = filSiyahKonumHareket[i][2] = filSiyahKonumGecici[i][2];

	kaleBeyazKonum[i][0] = kaleBeyazKonumHareket[i][0] = kaleBeyazKonumGecici[i][0];
	kaleBeyazKonum[i][1] = kaleBeyazKonumHareket[i][1] = kaleBeyazKonumGecici[i][1];
	kaleBeyazKonum[i][2] = kaleBeyazKonumHareket[i][2] = kaleBeyazKonumGecici[i][2];

	kaleSiyahKonum[i][0] = kaleSiyahKonumHareket[i][0] = kaleSiyahKonumGecici[i][0];
	kaleSiyahKonum[i][1] = kaleSiyahKonumHareket[i][1] = kaleSiyahKonumGecici[i][1];
	kaleSiyahKonum[i][2] = kaleSiyahKonumHareket[i][2] = kaleSiyahKonumGecici[i][2];
}

vezirBeyazKonum[0] = vezirBeyazKonumHareket[0] = vezirBeyazKonumGecici[0];
vezirBeyazKonum[1] = vezirBeyazKonumHareket[1] = vezirBeyazKonumGecici[1];
vezirBeyazKonum[2] = vezirBeyazKonumHareket[2] = vezirBeyazKonumGecici[2];

vezirSiyahKonum[0] = vezirSiyahKonumHareket[0] = vezirSiyahKonumGecici[0];
vezirSiyahKonum[1] = vezirSiyahKonumHareket[1] = vezirSiyahKonumGecici[1];
vezirSiyahKonum[2] = vezirSiyahKonumHareket[2] = vezirSiyahKonumGecici[2];

sahBeyazKonum[0] = sahBeyazKonumHareket[0] = sahBeyazKonumGecici[0];
sahBeyazKonum[1] = sahBeyazKonumHareket[1] = sahBeyazKonumGecici[1];
sahBeyazKonum[2] = sahBeyazKonumHareket[2] = sahBeyazKonumGecici[2];

sahSiyahKonum[0] = sahSiyahKonumHareket[0] = sahSiyahKonumGecici[0];
sahSiyahKonum[1] = sahSiyahKonumHareket[1] = sahSiyahKonumGecici[1];
sahSiyahKonum[2] = sahSiyahKonumHareket[2] = sahSiyahKonumGecici[2];

RenderPointerHamleReset();

//Yenen taslarýn indislerinide baslangýcla ayný yaptýk
yenenBeyazX = 8;
yenenSiyahX = 8;
yenenBeyazZ = 8;
yenenSiyahZ = 9;
//------------------

//Beyaz taslar icin hanlesi olan taslarý goster
HamleOlanTaslariGoster();
//-------------------------
}