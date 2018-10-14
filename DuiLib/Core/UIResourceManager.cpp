#include "StdAfx.h"
#include "UIResourceManager.h"

namespace DuiLib {

	CResourceManager::CResourceManager (void) {
		m_pQuerypInterface = nullptr;

	}

	CResourceManager::~CResourceManager (void) {
		ResetResourceMap ();
	}

	BOOL CResourceManager::LoadResource (std::variant<UINT, string_t> xml, string_view_t type) {
		if (xml.index () == 1) {
			if (std::get<1> (xml)[0] == _T ('<')) {
				if (!m_xml.Load (std::get<1> (xml).data ())) return FALSE;
			} else {
				if (!m_xml.LoadFromFile (std::get<1> (xml).data ())) return FALSE;
			}
		} else {
			HRSRC hResource = ::FindResource (CPaintManagerUI::GetResourceDll (), MAKEINTRESOURCE (std::get<0> (xml)), type.data ());
			if (hResource == nullptr) return FALSE;
			HGLOBAL hGlobal = ::LoadResource (CPaintManagerUI::GetResourceDll (), hResource);
			if (hGlobal == nullptr) {
				FreeResource (hResource);
				return FALSE;
			}

			if (!m_xml.LoadFromMem ((BYTE*) ::LockResource (hGlobal), ::SizeofResource (CPaintManagerUI::GetResourceDll (), hResource))) {
				return FALSE;
			}
			::FreeResource (hResource);
		}

		return LoadResource (m_xml.GetRoot ());
	}

	BOOL CResourceManager::LoadResource (CMarkupNode Root) {
		if (!Root.IsValid ()) return FALSE;

		LPCTSTR pstrClass = nullptr;
		int nAttributes = 0;
		LPCTSTR pstrName = nullptr;
		LPCTSTR pstrValue = nullptr;
		LPTSTR pstr = nullptr;

		//加载图片资源
		LPCTSTR pstrId = nullptr;
		LPCTSTR pstrPath = nullptr;
		for (CMarkupNode node = Root.GetChild (); node.IsValid (); node = node.GetSibling ()) {
			pstrClass = node.GetName ();
			CMarkupNode ChildNode = node.GetChild ();
			if (ChildNode.IsValid ()) LoadResource (node);
			else if ((_tcsicmp (pstrClass, _T ("Image")) == 0) && node.HasAttributes ()) {
				//加载图片资源
				nAttributes = node.GetAttributeCount ();
				for (int i = 0; i < nAttributes; i++) {
					pstrName = node.GetAttributeName (i);
					pstrValue = node.GetAttributeValue (i);

					if (_tcsicmp (pstrName, _T ("id")) == 0) {
						pstrId = pstrValue;
					} else if (_tcsicmp (pstrName, _T ("path")) == 0) {
						pstrPath = pstrValue;
					}
				}
				if (pstrId == nullptr || pstrPath == nullptr) continue;
				CDuiString * pstrFind = static_cast<CDuiString *>(m_mImageHashMap.Find (pstrId));
				if (pstrFind != nullptr) continue;
				m_mImageHashMap.Insert (pstrId, (LPVOID)new CDuiString (pstrPath));
			} else if (_tcsicmp (pstrClass, _T ("Xml")) == 0 && node.HasAttributes ()) {
				//加载XML配置文件
				nAttributes = node.GetAttributeCount ();
				for (int i = 0; i < nAttributes; i++) {
					pstrName = node.GetAttributeName (i);
					pstrValue = node.GetAttributeValue (i);

					if (_tcsicmp (pstrName, _T ("id")) == 0) {
						pstrId = pstrValue;
					} else if (_tcsicmp (pstrName, _T ("path")) == 0) {
						pstrPath = pstrValue;
					}
				}
				if (pstrId == nullptr || pstrPath == nullptr) continue;
				CDuiString * pstrFind = static_cast<CDuiString *>(m_mXmlHashMap.Find (pstrId));
				if (pstrFind != nullptr) continue;
				m_mXmlHashMap.Insert (pstrId, (LPVOID)new CDuiString (pstrPath));
			} else continue;
		}
		return TRUE;
	}

	string_view_t CResourceManager::GetImagePath (string_view_t lpstrId) {
		CDuiString * lpStr = static_cast<CDuiString *>(m_mImageHashMap.Find (lpstrId));
		return lpStr == nullptr ? nullptr : lpStr->c_str ();
	}

	string_view_t CResourceManager::GetXmlPath (string_view_t lpstrId) {
		CDuiString * lpStr = static_cast<CDuiString *>(m_mXmlHashMap.Find (lpstrId));
		return lpStr == nullptr ? nullptr : lpStr->c_str ();
	}

	void CResourceManager::ResetResourceMap () {
		for (int i = 0; i < m_mImageHashMap.GetSize (); i++) {
			string_view_t key = m_mImageHashMap.GetAt (i);
			if (!key.empty ())
				delete static_cast<CDuiString *>(m_mImageHashMap.Find (key));
		}
		for (int i = 0; i < m_mXmlHashMap.GetSize (); i++) {
			string_view_t key = m_mXmlHashMap.GetAt (i);
			if (!key.empty ())
				delete static_cast<CDuiString *>(m_mXmlHashMap.Find (key));
		}
		for (int i = 0; i < m_mTextResourceHashMap.GetSize (); i++) {
			string_view_t key = m_mTextResourceHashMap.GetAt (i);
			if (!key.empty ())
				delete static_cast<CDuiString *>(m_mTextResourceHashMap.Find (key));
		}
	}

	BOOL CResourceManager::LoadLanguage (string_view_t pstrXml) {
		CMarkup xml;
		if (pstrXml[0] == _T ('<')) {
			if (!xml.Load (pstrXml.data ())) return FALSE;
		} else {
			if (!xml.LoadFromFile (pstrXml.data ())) return FALSE;
		}
		CMarkupNode Root = xml.GetRoot ();
		if (!Root.IsValid ()) return FALSE;

		LPCTSTR pstrClass = nullptr;
		int nAttributes = 0;
		LPCTSTR pstrName = nullptr;
		LPCTSTR pstrValue = nullptr;
		LPTSTR pstr = nullptr;

		//加载图片资源
		LPCTSTR pstrId = nullptr;
		LPCTSTR pstrText = nullptr;
		for (CMarkupNode node = Root.GetChild (); node.IsValid (); node = node.GetSibling ()) {
			pstrClass = node.GetName ();
			if ((_tcsicmp (pstrClass, _T ("Text")) == 0) && node.HasAttributes ()) {
				//加载图片资源
				nAttributes = node.GetAttributeCount ();
				for (int i = 0; i < nAttributes; i++) {
					pstrName = node.GetAttributeName (i);
					pstrValue = node.GetAttributeValue (i);

					if (_tcsicmp (pstrName, _T ("id")) == 0) {
						pstrId = pstrValue;
					} else if (_tcsicmp (pstrName, _T ("value")) == 0) {
						pstrText = pstrValue;
					}
				}
				if (pstrId == nullptr || pstrText == nullptr) continue;

				CDuiString *lpstrFind = static_cast<CDuiString *>(m_mTextResourceHashMap.Find (pstrId));
				if (lpstrFind != nullptr) {
					*lpstrFind = pstrText;
				} else {
					m_mTextResourceHashMap.Insert (pstrId, (LPVOID)new CDuiString (pstrText));
				}
			} else continue;
		}

		return TRUE;
	}

	CDuiString CResourceManager::GetText (string_view_t lpstrId, string_view_t lpstrType) {
		if (lpstrId == nullptr) return _T ("");

		CDuiString *lpstrFind = static_cast<CDuiString *>(m_mTextResourceHashMap.Find (lpstrId));
		if (lpstrFind == nullptr && m_pQuerypInterface) {
			lpstrFind = new CDuiString (m_pQuerypInterface->QueryControlText (lpstrId, lpstrType));
			m_mTextResourceHashMap.Insert (lpstrId, (LPVOID) lpstrFind);
		}
		return lpstrFind == nullptr ? lpstrId : *lpstrFind;
	}

	void CResourceManager::ReloadText () {
		if (m_pQuerypInterface == nullptr) return;
		//重载文字描述
		for (int i = 0; i < m_mTextResourceHashMap.GetSize (); i++) {
			string_view_t lpstrId = m_mTextResourceHashMap.GetAt (i);
			if (lpstrId == nullptr) continue;
			string_view_t lpstrText = m_pQuerypInterface->QueryControlText (lpstrId, nullptr);
			if (lpstrText != nullptr) {
				CDuiString *lpStr = static_cast<CDuiString*> (m_mTextResourceHashMap.Find (lpstrId));
				*lpStr = lpstrText;
			}
		}
	}
	void CResourceManager::ResetTextMap () {
		CDuiString * lpStr;
		for (int i = 0; i < m_mTextResourceHashMap.GetSize (); i++) {
			string_view_t key = m_mTextResourceHashMap.GetAt (i);
			if (!key.empty ()) {
				lpStr = static_cast<CDuiString *>(m_mTextResourceHashMap.Find (key));
				delete lpStr;
			}
		}
	}


} // namespace DuiLib