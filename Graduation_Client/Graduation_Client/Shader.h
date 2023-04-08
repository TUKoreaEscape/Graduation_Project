#pragma once

class Shader
{
public:
	Shader();
	virtual ~Shader();

private:
	int									m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState = 0);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int nPipelineState = 0);
	virtual D3D12_BLEND_DESC CreateBlendState(int nPipelineState = 0);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState = 0);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(int nPipelineState = 0);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(int nPipelineState = 0);

	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFromFile(WCHAR* pszFileName, ID3DBlob** ppd3dShaderBlob = NULL);

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nPipelineState = 0);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void ReleaseShaderVariables() { }

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World) { }

	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);

	virtual void ReleaseUploadBuffers() { }

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL) { }
	virtual void AnimateObjects(float fTimeElapsed) { }
	virtual void ReleaseObjects() { }

protected:
	ID3DBlob* m_pd3dVertexShaderBlob = NULL;
	ID3DBlob* m_pd3dPixelShaderBlob = NULL;

	int m_nPipelineStates = 0;
	ID3D12PipelineState** m_ppd3dPipelineStates = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_d3dPipelineStateDesc;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
class TerrainShader : public Shader
{
public:
	TerrainShader();
	virtual ~TerrainShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState = 0);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(int nPipelineState = 0);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(int nPipelineState = 0);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class SkyBoxShader : public Shader
{
public:
	SkyBoxShader();
	virtual ~SkyBoxShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState = 0);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int nPipelineState = 0);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(int nPipelineState = 0);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(int nPipelineState = 0);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class StandardShader : public Shader
{
public:
	StandardShader();
	virtual ~StandardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState = 0);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(int nPipelineState = 0);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(int nPipelineState = 0);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class WallShader : public Shader
{
public:
	WallShader();
	virtual ~WallShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState = 0);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(int nPipelineState = 0);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(int nPipelineState = 0);

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class PlayerShader : public StandardShader
{
public:
	PlayerShader();
	virtual ~PlayerShader();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class SkinnedAnimationShader : public StandardShader
{
public:
	SkinnedAnimationShader();
	virtual ~SkinnedAnimationShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int nPipelineState = 0);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(int nPipelineState = 0);
};