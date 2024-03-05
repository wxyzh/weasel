module;
#include "stdafx.h"
module WeaselTSF;
import Composition;
import CandidateList;
import WeaselUtility;
// import TransitoryExtension;

void WeaselTSF::_StartComposition(ITfContext* pContext, bool not_inline_preedit)
{
	com_ptr<CStartCompositionEditSession> pStartCompositionEditSession;
	pStartCompositionEditSession.Attach(new CStartCompositionEditSession(this, pContext, not_inline_preedit));
	if (pStartCompositionEditSession != nullptr)
	{
		HRESULT hr{ S_OK };
		auto ret = pContext->RequestEditSession(_tfClientId, pStartCompositionEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
		if (!GetBit(WeaselFlag::DONT_DESTROY_UI))
			_cand->StartUI();
		else
			ResetBit(WeaselFlag::DONT_DESTROY_UI);
		SetBit(WeaselFlag::FIRST_KEY_COMPOSITION);
		if (SUCCEEDED(ret) && FAILED(hr) && !GetBit(WeaselFlag::COMPOSITION_FAILED))
		{
			m_client.ClearComposition();
			_cand->Destroy();
			_FinalizeComposition();
			SetBit(WeaselFlag::COMPOSITION_FAILED);
			RetryFailedEvent();
		}
	}
}

void WeaselTSF::_EndComposition(ITfContext* pContext, BOOL clear)
{
	com_ptr<CEndCompositionEditSession> pEditSession;
	pEditSession.Attach(new CEndCompositionEditSession(this, pContext, _pComposition, clear));
	HRESULT hr;

	if (pEditSession != nullptr)
	{
		pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
	}
	if (GetBit(WeaselFlag::DONT_DESTROY_UI) && SUCCEEDED(hr))
	{
		return;
	}
	// after pEditSession, less flicker
	_cand->EndUI();
	ResetBit(WeaselFlag::DONT_DESTROY_UI);
	ResetBit(WeaselFlag::COMPOSITION_FAILED);
}

/* Composition Window Handling */
BOOL WeaselTSF::_UpdateCompositionWindow(ITfContext* pContext)
{
	if (GetBit(WeaselFlag::NOT_INLINE_PREEDIT_LOST_FIRST_KEY))
	{
		_AbortComposition(true);
		return TRUE;
	}	
	com_ptr<ITfContextView> pContextView;
	HRESULT hr;
	hr = pContext->GetActiveView(&pContextView);
	if (FAILED(hr))
		return FALSE;
	com_ptr<CGetTextExtentEditSession> pEditSession;
	pEditSession.Attach(new CGetTextExtentEditSession(this, pContext, pContextView, _pComposition));
	if (pEditSession == NULL)
	{
		return FALSE;
	}

	auto ret = pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READ, &hr);
	return SUCCEEDED(ret);
}

void WeaselTSF::_SetCompositionPosition(const RECT& rc)
{
	/* Test if rect is valid.
	 * If it is invalid during CUAS test, we need to apply CUAS workaround */
	static RECT rect{};
	if (!_fCUASWorkaroundTested)
	{
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		_fCUASWorkaroundTested = true;
		if ((abs(rc.left - rect.left) < 2 && abs(rc.top - rect.top) < 2)
			|| (abs(rc.left - rect.right) < 5 && abs(rc.top - rect.bottom) < 5))
		{
			_fCUASWorkaroundEnabled = true;
		}
	}
	RECT rcCopy{ rc };
	if (rcCopy.left == 0 || abs(rcCopy.right - rect.right) < 5)
	{
		rcCopy.left = rcCopy.right = rect.right / 3.0;
		rcCopy.top = rcCopy.bottom = rect.bottom / 3.0 * 2;
	}
	m_client.UpdateInputPosition(rcCopy);
	_cand->UpdateInputPosition(rcCopy);
}

BOOL WeaselTSF::_ShowInlinePreedit(ITfContext* pContext, const std::shared_ptr<weasel::Context> context)
{
	com_ptr<CInlinePreeditEditSession> pEditSession;
	pEditSession.Attach(new CInlinePreeditEditSession(this, pContext, _pComposition, context));
	if (pEditSession != nullptr)
	{
		HRESULT hr;
		pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
	}

	return TRUE;
}

BOOL WeaselTSF::_InsertText(ITfContext* pContext, std::wstring_view text)
{
	com_ptr<CInsertTextEditSession> pEditSession;
	pEditSession.Attach(new CInsertTextEditSession(this, pContext, _pComposition, text));
	HRESULT hr;

	if (pEditSession != nullptr)
	{
		pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
	}
	return TRUE;
}

void WeaselTSF::_UpdateComposition(ITfContext* pContext)
{
	HRESULT hr;
	_pEditSessionContext = pContext;
	// _pEditSessionContext = TransitoryExtension::ToParentContextIfExists(pContext);
	ResetBit(WeaselFlag::ASYNC_EDIT);
	auto ret = _pEditSessionContext->RequestEditSession(_tfClientId, this, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
	if (hr == TF_S_ASYNC)
		SetBit(WeaselFlag::ASYNC_EDIT);
}

/* Composition State */
STDAPI WeaselTSF::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition* pComposition)
{
	// NOTE:
	// This will be called when an edit session ended up with an empty composition string,
	// Even if it is closed normally.
	// Silly M$.
	_AbortComposition();

	if (!GetBit(WeaselFlag::FOCUS_CHANGED))
	{
		if (GetBit(WeaselFlag::FIREFOX) && GetBit(WeaselFlag::FIRST_KEY_COMPOSITION) && !GetBit(WeaselFlag::PREDICTION))	// 重发意外终止的首码事件
		{
			SetBit(WeaselFlag::RETRY_COMPOSITION);
			RetryKey();
		}
	}

	return S_OK;
}

void WeaselTSF::_AbortComposition(bool clear)
{
	BOOL eaten{};
	if (GetBit(WeaselFlag::INLINE_PREEDIT_LOST_FIRST_KEY))
	{
		ResetBit(WeaselFlag::INLINE_PREEDIT_LOST_FIRST_KEY);
		SetBit(WeaselFlag::DONT_DESTROY_UI);
		_EndComposition(_pEditSessionContext, false);
		if (m_preeditCount > 1)
		{			
			_ProcessKeyEvent(VK_BACK, 0x1, &eaten);
		}
		return;
	}

	m_client.ClearComposition();
	if (_IsComposing()) {
		if (GetBit(WeaselFlag::NOT_INLINE_PREEDIT_LOST_FIRST_KEY))
		{
			ResetBit(WeaselFlag::NOT_INLINE_PREEDIT_LOST_FIRST_KEY);
			_ProcessKeyEvent(_lastKey, 0x1, &eaten);
			SetBit(WeaselFlag::DONT_DESTROY_UI);
		}
		_EndComposition(_pEditSessionContext, clear);
		return;
	}
	_cand->Destroy();	
}

void WeaselTSF::_FinalizeComposition()
{
	_pComposition = nullptr;
}

void WeaselTSF::_SetComposition(ITfComposition* pComposition)
{
	_pComposition = pComposition;
}

BOOL WeaselTSF::_IsComposing()
{
	return _pComposition != NULL;
}

bool WeaselTSF::RetryKey()
{
	unsigned send{};
	std::array<INPUT, 1> inputs;

	inputs[0].type = INPUT_KEYBOARD;
	inputs[0].ki = { _lastKey };

	send = SendInput(inputs.size(), inputs.data(), sizeof(INPUT));

	return send == 1;
}

// 获取程序当前所在显示器的分辨率大小，可以动态的获取程序所在显示器的分辨率
SIZE WeaselTSF::GetScreenResolution()
{
	SIZE size{};
	if (!m_hwnd)
		return size;

	// MONITOR_DEFAULTTONEAREST 返回值是最接近该点的屏幕句柄
	// MONITOR_DEFAULTTOPRIMARY 返回值是主屏幕的句柄
	// 如果其中一个屏幕包含该点，则返回值是该屏幕的HMONITOR句柄。如果没有一个屏幕包含该点，则返回值取决于dwFlags的值
	HMONITOR hMonitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFOEX miex;
	miex.cbSize = sizeof(miex);
	if (!GetMonitorInfo(hMonitor, &miex))
		return size;

	DEVMODE dm;
	dm.dmSize = sizeof(dm);
	dm.dmDriverExtra = 0;

	// ENUM_CURRENT_SETTINGS 检索显示设备的当前设置
	// ENUM_REGISTRY_SETTINGS 检索当前存储在注册表中的显示设备的设置
	if (!EnumDisplaySettings(miex.szDevice, ENUM_CURRENT_SETTINGS, &dm))
		return size;

	size.cx = dm.dmPelsWidth;
	size.cy = dm.dmPelsHeight;
	return size;
}

void WeaselTSF::RetryFailedEvent()
{
	SIZE size = GetScreenResolution();
	if (size.cx == 0 && size.cy == 0)
		return;

	std::array<INPUT, 5> inputs;
	POINT ptCursor{};
	GetCursorPos(&ptCursor);

	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dx = m_rcFallback.left / static_cast<double>(size.cx) * 65535;
	inputs[0].mi.dy = m_rcFallback.top / static_cast<double>(size.cy) * 65535;
	inputs[0].mi.mouseData = 0;
	inputs[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	inputs[0].mi.time = 0;

	inputs[1].type = INPUT_MOUSE;
	inputs[1].mi.mouseData = 0;
	inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	inputs[1].mi.time = 0;

	inputs[2].type = INPUT_MOUSE;
	inputs[2].mi.mouseData = 0;
	inputs[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;
	inputs[2].mi.time = 0;

	inputs[3].type = INPUT_MOUSE;
	inputs[3].mi.dx = ptCursor.x / static_cast<double>(size.cx) * 65535;
	inputs[3].mi.dy = ptCursor.y / static_cast<double>(size.cy) * 65535;
	inputs[3].mi.mouseData = 0;
	inputs[3].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	inputs[3].mi.time = 0;

	inputs[4].type = INPUT_KEYBOARD;
	inputs[4].ki = { _lastKey, _lastKey, 0, 0, 0 };

	SendInput(inputs.size(), inputs.data(), sizeof(INPUT));
}

void WeaselTSF::SimulatingKeyboardEvents(unsigned short code)
{
	std::array<INPUT, 2> inputs;

	inputs[0].type = INPUT_KEYBOARD;
	inputs[0].ki = { code };

	inputs[1].type = INPUT_KEYBOARD;
	inputs[1].ki = { code, 0, KEYEVENTF_KEYUP };

	SendInput(inputs.size(), inputs.data(), sizeof(INPUT));
}

void WeaselTSF::SetRect(const RECT& rc)
{ 
	m_rcFallback = rc;
	if (!GetBit(WeaselFlag::CARET_FOLLOWING))
	{
		WriteConfiguration(ConfigFlag::FALLBACK_POSITION);
	}
}