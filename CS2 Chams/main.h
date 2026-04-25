
//https://github.com/Enoticx1/CS2-DirectX-Chams

HWND window = nullptr;
BOOL bInit = FALSE;
bool bSetPos = false;
ImVec2 vWindowPos = { 0, 0 };
ImVec2 vWindowSize = { 0, 0 };

#define SAFE_RELEASE(x) if (x) { x->Release(); x = NULL; }

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;



bool showmenu = false;
bool initonce = false;


bool wallhack = 1;
bool chams = 0;
bool teamchams = 0;
float chams_alpha = 1.0f;
ID3D11BlendState* TransparencyState = NULL;

float chams_color_visible[3] = { 0.0f, 1.0f, 0.0f };
float chams_color_hidden[3] = { 1.0f, 0.0f, 0.0f };
float chams_color_hands[3] = { 1.0f, 1.0f, 1.0f };
int render_mode = 0;

int check_draw_result = 0;
int check_drawindexed_result = 0;
int check_drawindexedinstanced_result = 0;


ID3D11RenderTargetView* mainRenderTargetViewD3D11;


UINT stencilRef = 0;
ID3D11DepthStencilState* DepthStencilState_TRUE = NULL;
ID3D11DepthStencilState* DepthStencilState_FALSE = NULL;
ID3D11DepthStencilState* DepthStencilState_ORIG = NULL;


ID3D11RasterizerState* OldState;
ID3D11RasterizerState* CustomState = NULL;
ID3D11RasterizerState* WireframeState = NULL;


ID3D11PixelShader* sRed = NULL;
ID3D11PixelShader* sRedDark = NULL;
ID3D11PixelShader* sGreen = NULL;
ID3D11PixelShader* sGreenDark = NULL;
ID3D11PixelShader* sBlue = NULL;
ID3D11PixelShader* sYellow = NULL;
ID3D11PixelShader* sMagenta = NULL;
ID3D11PixelShader* sGrey = NULL;

ID3D11PixelShader* sUniformColorVisible = NULL;
ID3D11PixelShader* sUniformColorHidden = NULL;
ID3D11PixelShader* sUniformColorHands = NULL;

ID3D11PixelShader* oPixelShaderA;




using namespace std;
#include <fstream>


char dlldir[320];
char* GetDirFile(char* name)
{
	static char pldir[320];
	strcpy_s(pldir, dlldir);
	strcat_s(pldir, name);
	return pldir;
}


void Log(const char* fmt, ...)
{
	if (!fmt)	return;

	char		text[4096];
	va_list		ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);

	ofstream logfile((PCHAR)"log.txt", ios::app);
	if (logfile.is_open() && text)	logfile << text << endl;
	logfile.close();
}



std::string GetDebugName(ID3D11DeviceChild* pObject)
{
	if (!pObject)
		return {};

	UINT dataSize = 0;

	HRESULT hResult = pObject->GetPrivateData(WKPDID_D3DDebugObjectName, &dataSize, nullptr);
	if (FAILED(hResult) || dataSize == 0)
		return {};

	std::vector<char> name(dataSize + 1);
	UINT size = dataSize;

	hResult = pObject->GetPrivateData(WKPDID_D3DDebugObjectName, &size, name.data());
	if (SUCCEEDED(hResult))
		return std::string(name.data());

	return {};
}



#include <D3Dcompiler.h>
#pragma comment(lib, "D3dcompiler.lib")

HRESULT GenerateShader(ID3D11Device* pDevice, ID3D11PixelShader** pShader, float r, float g, float b)
{
	static const char szCast[] =
		"float4 main(float4 color: COLOR0): SV_Target0 {\n"
		"float4 col;"
		" col.a = 0.5f;"
		" col.r = %f;"
		" col.g = %f;"
		" col.b = %f;"
		"  return col;\n"
		"}\n";

	ID3D10Blob* pBlob;
	char szPixelShader[1000];

	sprintf_s(szPixelShader, szCast, r, g, b);

	ID3DBlob* error;

	HRESULT hr = D3DCompile(szPixelShader, sizeof(szPixelShader), "shader", NULL, NULL, "main", "ps_4_0", NULL, NULL, &pBlob, &error);

	if (FAILED(hr))
		return hr;

	hr = pDevice->CreatePixelShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, pShader);

	if (FAILED(hr))
		return hr;

	return S_OK;
}



#include <string>
#include <fstream>
void SaveCfg()
{
	ofstream fout;
	fout.open(GetDirFile((PCHAR)"cs2d11.ini"), ios::trunc);
	fout << "Wallhack " << wallhack << endl;
	fout << "Chams " << chams << endl;
	fout << "TeamChams " << teamchams << endl;
	fout << "ChamsColorVisibleR " << chams_color_visible[0] << endl;
	fout << "ChamsColorVisibleG " << chams_color_visible[1] << endl;
	fout << "ChamsColorVisibleB " << chams_color_visible[2] << endl;
	fout << "ChamsColorHiddenR " << chams_color_hidden[0] << endl;
	fout << "ChamsColorHiddenG " << chams_color_hidden[1] << endl;
	fout << "ChamsColorHiddenB " << chams_color_hidden[2] << endl;
	fout << "ChamsColorHandsR " << chams_color_hands[0] << endl;
	fout << "ChamsColorHandsG " << chams_color_hands[1] << endl;
	fout << "ChamsColorHandsB " << chams_color_hands[2] << endl;
	fout << "RenderMode " << render_mode << endl;
	fout.close();
}

void LoadCfg()
{
	ifstream fin;
	string Word = "";
	fin.open(GetDirFile((PCHAR)"cs2d11.ini"), ifstream::in);
	fin >> Word >> wallhack;
	fin >> Word >> chams;
	fin >> Word >> teamchams;
	fin >> Word >> chams_color_visible[0];
	fin >> Word >> chams_color_visible[1];
	fin >> Word >> chams_color_visible[2];
	fin >> Word >> chams_color_hidden[0];
	fin >> Word >> chams_color_hidden[1];
	fin >> Word >> chams_color_hidden[2];
	fin >> Word >> chams_color_hands[0];
	fin >> Word >> chams_color_hands[1];
	fin >> Word >> chams_color_hands[2];
	fin >> Word >> render_mode;
	fin.close();
}
