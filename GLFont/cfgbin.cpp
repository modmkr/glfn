
#include <windows.h>
#include <stdio.h>

#include "cfgbin.h"

char* AllocString(const char* pszSrc);


class CCfgValue
{
public:
	CCfgValue* pNext;
	CCfgValue(char* pszName, int eType, void* p);
	~CCfgValue();
	void Load(FILE* stream);
	void Save(FILE* stream);
private:
	char* m_pszName;
	int m_eType;
	void* m_p;
};

CCfgValue::CCfgValue(char* pszName, int eType, void* p)
{
	pNext = NULL;
	m_pszName = AllocString(pszName);
	m_eType = eType;
	m_p = p;
}

CCfgValue::~CCfgValue()
{
	free(m_pszName);
}


void CCfgValue::Load(FILE* stream)
{
	int iSize;
	switch (m_eType)
	{
	case VAR_INT:
		fread(m_p, sizeof(int), 1, stream);
		break;
	case VAR_FLOAT:
		fread(m_p, sizeof(float), 1, stream);
		break;
	case VAR_BOOL:
		fread(m_p, sizeof(bool), 1, stream);
		break;
	case VAR_SZ:
		fread(&iSize, sizeof(int), 1, stream);
		fread(m_p, iSize, 1, stream);
		break;
	case VAR_SZ_PTR:
		fread(&iSize, sizeof(int), 1, stream);
		if (*(char**)m_p != NULL)
		{
			free(*(char**)m_p);
			*(char**)m_p = NULL;
		}
		if (iSize != 0)
		{
			*(char**)m_p = (char*)malloc(iSize);
			fread(*(char**)m_p, iSize, 1, stream);
		}
		break;
	}
}


void CCfgValue::Save(FILE* stream)
{
	int iSize;
	switch (m_eType)
	{
	case VAR_INT:
		fwrite(m_p, sizeof(int), 1, stream);
		break;
	case VAR_FLOAT:
		fwrite(m_p, sizeof(float), 1, stream);
		break;
	case VAR_BOOL:
		fwrite(m_p, sizeof(bool), 1, stream);
		break;
	case VAR_SZ:
		iSize = strlen((char*)m_p) + 1;
		fwrite(&iSize, sizeof(int), 1, stream);
		fwrite(m_p, iSize, 1, stream);
		break;
	case VAR_SZ_PTR:
		if (*(char**)m_p != NULL)
		{
			iSize = strlen(*(char**)m_p) + 1;
		}
		else
		{
			iSize = 0;
		}
		fwrite(&iSize, sizeof(int), 1, stream);
		if (iSize != 0)
		{
			fwrite(*(char**)m_p, iSize, 1, stream);
		}
		break;
	}
}


CCfgFile::CCfgFile(void)
{
	m_pFirst = NULL;
}


CCfgFile::~CCfgFile()
{
	CCfgValue* pValue = m_pFirst;
	while (pValue != NULL)
	{
		CCfgValue* pNext = pValue->pNext;
		delete pValue;
		pValue = pNext;
	}
}


void CCfgFile::AddValue(char* pszName, int eType, void* p)
{
	CCfgValue* pValue = new CCfgValue(pszName, eType, p);
	
	if (m_pFirst != NULL)
	{
		pValue->pNext = m_pFirst;
	}

	m_pFirst = pValue;
}


void CCfgFile::Load(FILE* stream)
{
	CCfgValue* pValue = m_pFirst;
	while (pValue != NULL)
	{
		pValue->Load(stream);
		pValue = pValue->pNext;
	}
}


void CCfgFile::Save(FILE* stream)
{
	CCfgValue* pValue = m_pFirst;
	while (pValue != NULL)
	{
		pValue->Save(stream);
		pValue = pValue->pNext;
	}
}

