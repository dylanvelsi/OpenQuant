#ifndef _CA_API_H
#define _CA_API_H

#include "CMHelper.h"
#include "CommonAssist.h"
#include <vector> 
#include <shlobj.h>
#include <math.h> 
#include "CA_Lock.h"

_CA_BEGIN


#ifndef _vect2Ptr  
	#define _vect2Ptr(v)  ((v).size() == 0? NULL :&(v)[0]) 
#endif 

#ifndef _vectIntSize
	#define _vectIntSize(v)  ((int)v.size())
#endif 


#ifndef _vectIntSize 
	#define  _vectIntSize(v) ((int)v.size()) 
#endif 

#ifndef _countof
	#define _countof(array)  (sizeof(array)/sizeof(array[0]))
#endif



static void  Unicode2UTF(const WCHAR*wcsCnt,std::string&strRet) 
{
	// Convert from UNICODE (AKA UCS-2) to UTF-8
	//获取转换后的长度 
	int nStrLen = wcslen(wcsCnt); 
	strRet.clear(); 
	if(nStrLen > 0)
	{
		int nUtf8Length = WideCharToMultiByte(CP_UTF8
			, 0
			, wcsCnt
			, nStrLen
			, NULL
			, 0
			, NULL
			, FALSE);

		// Allocate Buffer for UTF-8 file data 

		strRet.resize(nUtf8Length);  
		int nLength = WideCharToMultiByte(CP_UTF8
			, 0 
			, wcsCnt
			, nStrLen
			, (LPSTR)strRet.c_str()
			, nUtf8Length
			, NULL
			, FALSE);
		ATLASSERT( nLength == nUtf8Length );
	}
}


static void  UTF2Unicode(LPCSTR pstr, std::wstring & strContext)
{
	static CA::CCriticalSection  s_lock;
	static std::vector<wchar_t> s_vtTemp;   

	CA::CAutoLock<CA::CCriticalSection>  lock(&s_lock);

	int nStrLen = pstr? strlen(pstr):0; 
	strContext.clear(); 

	if(nStrLen > 0)
	{

		int nUnicodeLength = 2*nStrLen+1; 

		// 减少资源开销 [6/26/2012 ysq]
		/* 
		MultiByteToWideChar(CP_UTF8
			, 0
			, pstr
		, nStrLen
			, NULL
			, 0);
		*/ 
		// Allocate Buffer for UNICODE file data
		if ((int)s_vtTemp.size() < nUnicodeLength)
		{ 
			int nNewLen = (nUnicodeLength+1023)/1024 *1024; 
			s_vtTemp.resize(nNewLen); 
		}
		wchar_t * pszBuffer = _vect2Ptr(s_vtTemp); //new wchar_t[nUnicodeLength + 1]; 

		int nLength = MultiByteToWideChar(CP_UTF8
			, 0 
			, pstr
		, nStrLen
			, (LPWSTR)pszBuffer
			, nUnicodeLength); 

		//ATLASSERT( nLength == nUnicodeLength );

		//未尾置0 
		if (nLength >= 0 && nLength < nUnicodeLength) 
			pszBuffer[nLength] = 0; 
		else 
		{
			//ASSERT(false); 
		} 

		strContext = pszBuffer;
	}
}


static CString GetMoudleFolder(HINSTANCE hInstance = NULL)
{
	TCHAR szFilename[MAX_PATH+1] = {0};
	GetModuleFileName(hInstance, szFilename, MAX_PATH);

	CString strFilename(szFilename);
	int nPos = strFilename.ReverseFind(_T('\\'));

	return strFilename.Left(nPos + 1);
}

static CString MakeTempFileName(LPCTSTR lpPrefixStr, LPCTSTR pstrFileExt)
{
	TCHAR chPath[MAX_PATH] = {0}, chFile[MAX_PATH]={0};
	GetTempPath(_countof(chPath), chPath); 

	static UINT s_nUnique = 0; 
	if (0 == s_nUnique)
		s_nUnique = GetTickCount(); 
	
	GetTempFileName(chPath, lpPrefixStr, s_nUnique++, chFile); 
 
	CString strRet(chFile); 
	if (pstrFileExt)
		strRet += pstrFileExt; 

	return strRet; 
}

static void DeleteNumStrTailZero(CString &str)
{
	int nLen = str.GetLength();
	if (nLen && str.Find('.') != -1 ) 
	{ 	
		while('0' == str.GetAt(nLen-1))  
		{
			str.Delete(nLen - 1, 1);
			nLen--; 
		}
		if ('.' == str.GetAt(nLen-1))
		{
			str.Delete(nLen - 1, 1);
		}
	}
}

static BOOL DivStr(const CString& str, const CString &strDiv, std::vector<CString> &arDiv)
{
	if (str.GetLength() == 0 || strDiv.GetLength() == 0)
	{
		return false;
	}
	arDiv.clear();

	CString strTmp = str;
	LPCTSTR seps = (LPCTSTR)strDiv;
	TCHAR* pstrSrc = strTmp.GetBuffer(strTmp.GetLength());
	TCHAR* token = _tcstok(pstrSrc, seps);
	while (token)
	{
		arDiv.push_back(CString(token));
		token = _tcstok(NULL, seps);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
// 创建目录
static BOOL CreateDirectory(CString strPath)
{
	ASSERT_RET(!strPath.IsEmpty(), FALSE);

	if (strPath.Right(1) != _T("\\"))
	{
		//最后一位不是\则被认为路径带文件名
		int nPos = strPath.ReverseFind('\\');
		strPath = strPath.Left(nPos + 1);
	}

	int nPos = -1;
	while ((nPos = strPath.Find('\\', nPos + 1)) > 0)
	{
		CString strTemp = strPath.Left(nPos + 1);
		ASSERT_RET(!strTemp.IsEmpty(), FALSE);

		if (!PathFileExists(strTemp))
			ASSERT_RET(::CreateDirectory(strTemp, NULL), FALSE);
	}

	return TRUE;
}

_CA_END

#endif


