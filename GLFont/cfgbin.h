
enum CfgVarTypes
{
	VAR_INT = 0,
	VAR_FLOAT,
	VAR_BOOL,
	VAR_SZ,
	VAR_SZ_PTR, // requires (re-)initialization of the pointer
	NUM_VAR_TYPES
};

class CCfgValue;

class CCfgFile
{
public:
	CCfgFile(void);
	~CCfgFile();
	void AddValue(char* pszName, int eType, void* p);
	void Load(FILE* stream);
	void Save(FILE* stream);
private:
	CCfgValue* m_pFirst;
};

