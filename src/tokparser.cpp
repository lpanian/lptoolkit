#include <cstdlib>
#include <cstring>
#include <cctype>
#include "toolkit/tokparser.hh"

namespace lptk
{

////////////////////////////////////////////////////////////////////////////////
bool Tokenizer::Next(char* tok, int& tokType, size_t maxLen)
{
	const char* data = m_data;
	auto pos = m_pos;
	auto const maxSize = m_size;

	// skip to the first non space
	while(pos < maxSize && isspace(data[pos]) )
	{
		if(data[pos] == '\n') 
			++m_curLine;
		++pos;
	}
	if(pos == maxSize)
		return 0;

	// skip comments
	while(pos < maxSize && data[pos] == ';')
	{
		while(pos < maxSize && data[pos] != '\n')
			++pos;
		++m_curLine;
		while(pos < maxSize && isspace(data[pos]))
			++pos;
	}
	if(pos == maxSize)
		return 0;
	auto tokStart = pos;

	// read the token
	size_t posTok = 0;
	const size_t maxPosTok = maxLen - 1 ;
	int type = T_NONE;
	if(data[pos] == '"')
	{
		type = T_STRING;
		++pos;
		while( pos < maxSize && data[pos] != '"' )
		{
			if(posTok < maxPosTok)
				tok[posTok++] = data[pos];
			++pos;
		}

		if(pos < maxSize && data[pos] == '"')
			++pos;
	}
	else if(isdigit(data[pos]) || data[pos] == '-')
	{
		type = T_NUMERIC;
		// read a numeric value
		if(data[pos] == '-') {
			if(posTok < maxPosTok)
				tok[posTok++] = data[pos];
			++pos;
		}

		// before decimal
		while(pos < maxSize && isdigit(data[pos])) 
		{
			if(posTok < maxPosTok)
				tok[posTok++] = data[pos];
			++pos;
		}

		// actual decimal
		if(pos < maxSize && data[pos] == '.') {
			if(posTok < maxPosTok)
				tok[posTok++] = data[pos];
			++pos;

			// after decimal
			while(pos < maxSize && isdigit(data[pos])) 
			{
				if(posTok < maxPosTok)
					tok[posTok++] = data[pos];
				++pos;
			}
		}

		// exponent
		if(pos < maxSize && (data[pos] == 'e' || data[pos] == 'E'))
		{
			if(posTok < maxPosTok)
				tok[posTok++] = data[pos];
			++pos;
		
			// read a numeric value
			if(data[pos] == '-') {
				if(posTok < maxPosTok)
					tok[posTok++] = data[pos];
				++pos;
			}
			
			while(pos < maxSize && isdigit(data[pos])) 
			{
				if(posTok < maxPosTok)
					tok[posTok++] = data[pos];
				++pos;
			}
		}
	}
	else if(isalpha(data[pos]))
	{
		while( pos < maxSize && isalnum(data[pos]))
		{
			if(posTok < maxPosTok)
				tok[posTok++] = data[pos];
			++pos;
		}

		type = T_SYMBOL;
	}
	else // generic single character token, like "{"
	{
		while(posTok < maxSize && !isspace(data[pos]) 
				&& !isalnum(data[pos]))
		{
			if(posTok < maxPosTok)
				tok[posTok++] = data[pos];
			++pos;
		}
		type = T_TOK;
	}
	tok[posTok] = '\0';

	if(type == T_SYMBOL && 
			(StrCaseEqual("true", tok) || StrCaseEqual("false", tok)))
		type = T_BOOLEAN;

	m_pos = pos;
	if(pos > tokStart)
	{
		tokType = type;
		return true;
	}
	
	tokType = T_NONE;
	return false;
}

////////////////////////////////////////////////////////////////////////////////
TokParser::TokParser()
	: m_tokenizer(nullptr, 0)
	, m_token()
	, m_error(true)
	, m_eof(true)
{
}

TokParser::TokParser(const char* data, size_t size)
	: m_tokenizer(data, size)
	, m_token()
	, m_error(false)
	, m_eof(false)
{
	Consume();
}

void TokParser::Consume()
{
	if(m_eof) { m_error = true; return; }
	if(!m_tokenizer.Next(m_token, m_tokType, sizeof(m_token)))
	{
		m_token[0] = '\0';
		m_tokType = T_NONE;
		m_eof = true;
	}
}

int TokParser::GetString(char* buffer, int maxLen)
{
	if(m_eof) { m_error = true; return 0; }
	if(m_tokType != T_STRING)
	{
		m_error = true;
		Consume();
		return 0;
	}
	int i, len = maxLen - 1;
	for(i = 0; i < len; ++i)
		buffer[i] = m_token[i];
	buffer[i] = '\0';
	Consume();
	return i;
}

int TokParser::GetSymbol(char* buffer, int maxLen)
{
	if(m_eof) { m_error = true; return 0; }
	if(m_tokType != T_SYMBOL)
	{
		m_error = true;
		Consume();
		return 0;
	}
	int i, len = maxLen - 1;
	for(i = 0; i < len && m_token[i]; ++i)
		buffer[i] = m_token[i];
	buffer[i] = '\0';
	Consume();
	return i;
}

int TokParser::GetInt()
{
	if(m_eof) { m_error = true; return 0; }
	if(m_tokType != T_NUMERIC)
	{
		m_error = true;
		Consume();
		return 0;
	}

	int value = atoi(m_token);
	Consume();
	return value;
}

unsigned int TokParser::GetUInt()
{
	if(m_eof) { m_error = true; return 0U; }
	if(m_tokType != T_NUMERIC)
	{
		m_error = true;
		Consume();
		return 0U;
	}
	unsigned long longval = strtoul(m_token, nullptr, 10);
	unsigned int value = (unsigned int)longval;
	Consume();
	return value;
}

float TokParser::GetFloat()
{
	if(m_eof) { m_error = true; return 0.f; }
	if(m_tokType != T_NUMERIC)
	{
		m_error = true;
		Consume();
		return 0.f;
	}
	float value = (float)atof(m_token);
	Consume();
	// TODO: consider removing this and replacing with fprintf(out, "%1.8e", f)
	if(m_token[0] == ':')
	{
		Consume();
		char* endPtr;
		unsigned long rawValue = strtoul(m_token, &endPtr, 16);
		if(endPtr == m_token)
		{
			m_error = true;
			return 0.f;
		}
		unsigned int rawValue32 = (unsigned int)(rawValue);
		// TODO: correct endian-ness
		memcpy(&value, &rawValue32, sizeof(value));
		Consume();
	}
	return value;
}
	
double TokParser::GetDouble()
{
	if(m_eof) { m_error = true; return 0.0; }
	if(m_tokType != T_NUMERIC)
	{
		m_error = true;
		Consume();
		return 0.0;
	}
	double value = (double)atof(m_token);
	Consume();
	return value;
}

bool TokParser::GetBool()
{
	if(m_eof) { m_error = true; return false; }

	bool result = false;
	if(m_tokType != T_BOOLEAN)
	{
		result = false;
		m_error = true;
	}
	else
	{
		if(StrCaseEqual("true", m_token))
			result = true;
		else if(StrCaseEqual("false", m_token))
			result = false;
		else
			m_error = true;
	}	
	Consume();
	return result;
}

int TokParser::GetFlag(const TokFlagDef* def, int numFlags)
{
	if(m_eof) { m_error = true; return 0; }
	int value = 0;
	const char* curWord = m_token;
	const char* cursor = m_token;
	for(;;) {
		if(*cursor == '+' || !*cursor) {
			auto const len = cursor - curWord;
			for(int flag = 0; flag < numFlags; ++flag)
			{
				if(StrNCaseEqual(def[flag].m_name, curWord, static_cast<unsigned int>(len)))
				{
					value |= def[flag].m_flag;
					break;
				}
			}
			// warn here if flag not recognized?

			if(!*cursor)
				break;

			++cursor;
			curWord = cursor;
		}
		else
			++cursor;
	}
	Consume();
	return value;
}

int TokParser::GetEnum(const TokFlagDef* def, int numFlags)
{
	if(m_eof) { m_error = true; return 0; }

	for(int i = 0; i < numFlags; ++i)
	{
		if(StrCaseEqual(def[i].m_name, m_token))
		{
			Consume();
			return def[i].m_flag;
		}
	}
	Consume();
	return 0;
}

bool TokParser::ExpectTok(const char* str)
{
	if(m_eof) { m_error = true; return false; }
	if(m_tokType != T_TOK)
	{
		m_error = true;
		Consume();
		return 0;
	}
	bool result = strcmp(str, m_token) == 0;
	Consume();
	if(!result)
		m_error = true;
	return result;
}
	
bool TokParser::IsTok(const char* str)
{
	if(m_eof) { return false; }
	if(m_tokType != T_TOK)
		return false;
	bool result = strcmp(str, m_token) == 0;
	return result;
}
	
bool TokParser::IsTokType(int tokType)
{
	if(m_eof) { return false; }
	return tokType == m_tokType;
}

bool TokParser::Skip()
{
	if(m_eof) { return false; }
	Consume();
	return true;
}

bool TokParser::SkipBlock()
{
	ExpectTok("{");
	int depth = 1;
	while(Ok() && depth > 0) {
		if(IsTok("{"))
			++depth;
		if(IsTok("}"))
			--depth;
		Consume();
	}

	if(depth != 0)
	{
		m_error = true;
		return false;
	}
	return true;
}

bool TokParser::Ok() const
{
	return !m_error && !m_eof;
}

bool TokParser::Error() const { 
	return m_error;
}
	
DynAry<float> TokParser::GetFloatStream()
{
	DynAry<float> result;
	if(!Ok())
		return result;
	if(IsTok("{")) {
		ExpectTok("{");
		while(Ok() && !IsTok("}")) {
			result.push_back(GetFloat());
		}
		ExpectTok("}");
	} else {
		result.push_back(GetFloat());
	}
	return result;
}

DynAry<double> TokParser::GetDoubleStream()
{
	DynAry<double> result;
	if(!Ok())
		return result;
	if(IsTok("{")) {
		ExpectTok("{");
		while(Ok() && !IsTok("}")) {
			result.push_back(GetDouble());
		}
		ExpectTok("}");
	} else {
		result.push_back(GetDouble());
	}
	return result;
}

DynAry<bool> TokParser::GetBoolStream()
{
	DynAry<bool> result;
	if(!Ok())
		return result;
	if(IsTok("{")) {
		ExpectTok("{");
		while(Ok() && !IsTok("}")) {
			result.push_back(GetBool());
		}
		ExpectTok("}");
	} else {
		result.push_back(GetBool());
	}
	return result;
}
	
DynAry<int> TokParser::GetIntStream()
{
	DynAry<int> result;
	if(!Ok())
		return result;
	if(IsTok("{")) {
		ExpectTok("{");
		while(Ok() && !IsTok("}")) {
			result.push_back(GetInt());
		}
		ExpectTok("}");
	} else {
		result.push_back(GetInt());
	}
	return result;
}

DynAry<unsigned int> TokParser::GetUIntStream()
{
	DynAry<unsigned int> result;
	if(!Ok())
		return result;
	if(IsTok("{")) {
		ExpectTok("{");
		while(Ok() && !IsTok("}")) {
			result.push_back(GetUInt());
		}
		ExpectTok("}");
	} else {
		result.push_back(GetUInt());
	}
	return result;
}


////////////////////////////////////////////////////////////////////////////////
TokWriter::TokWriter(const char* filename)
	: m_fp(nullptr)
	, m_error(false)
	, m_emptyLine(true)
{
#ifdef USING_VS
	if(0 != fopen_s(&m_fp, filename, "w"))
		m_fp = nullptr;
#else
	m_fp = fopen(filename, "w");
#endif

	if(!m_fp)
	{
		m_error = true;
	}
}

TokWriter::~TokWriter()
{
	if(m_fp)
		fclose(m_fp);
}
	
bool TokWriter::Ok() const 
{
	return !m_error;
}
	
bool TokWriter::Error() const 
{ 
	return m_error; 
}

void TokWriter::Comment(const char* str)
{
	fprintf(m_fp, "%s; %s\n", 
		(m_emptyLine ? "" : " "), str);
	m_emptyLine = true;
}

void TokWriter::String(const char* str)
{
	const char* start = m_emptyLine ? "" : " ";
	fprintf(m_fp, "%s\"%s\"", start, str);
	m_emptyLine = false;
}

void TokWriter::Symbol(const char* str)
{
	if(isdigit(*str) || *str == '-')
		std::cerr << "Invalid symbol being written: " << str << std::endl;
	bool hasSpace = true;
	const char* cursor = str;
	while(*cursor) {
		if(isspace(*cursor))
		{
			hasSpace = true;
			break;
		}
		++cursor;
	}

	if(hasSpace)
		std::cerr << "Invaild symbol being written: \"" << str << "\"" << std::endl;

	const char* start = m_emptyLine ? "" : " ";
	fprintf(m_fp, "%s%s", start, str);
	m_emptyLine = false;
}


void TokWriter::Int(int value)
{
	fprintf(m_fp, "%s%d", 
		(m_emptyLine ? "" : " "), value);
	m_emptyLine = false;
}

void TokWriter::UInt(unsigned int value)
{
	fprintf(m_fp, "%s%u", 
		(m_emptyLine ? "" : " "), value);
	m_emptyLine = false;
}

void TokWriter::Float(float value)
{
//	unsigned int rawValue32 = 0;
//	memcpy(&rawValue32, &value, sizeof(rawValue32));
	fprintf(m_fp, "%s%1.8e", 
		(m_emptyLine ? "" : " "), value);//, rawValue32);
	m_emptyLine = false;
}

void TokWriter::Double(double value)
{
	fprintf(m_fp, "%s%1.16e", (m_emptyLine ? "" : " "), value);
	m_emptyLine = false;
}
	
void TokWriter::Bool(bool value)
{
	fprintf(m_fp, "%s%s", (m_emptyLine ? "" : " "), value ? "true" : "false");
	m_emptyLine = false;
}

void TokWriter::Nl()
{
	fprintf(m_fp, "\n");
	m_emptyLine = true;
}

void TokWriter::Flag(const TokFlagDef* def, int numFlags, int value)
{
	int emptyFlag = 1;
	if(!m_emptyLine) fprintf(m_fp, " ");
	int flag;
	for(flag = 0; flag < numFlags; ++flag)
	{
		if(value & def[flag].m_flag)
		{
			const char* plus = emptyFlag ? "" : "+";
			emptyFlag = 0;
			fprintf(m_fp, "%s%s", plus, def[flag].m_name);
		}
	}
	if(emptyFlag)
		fprintf(m_fp, "\"\"");
	m_emptyLine = false;
}

void TokWriter::Token(const char* tok)
{
	fprintf(m_fp, "%s%s",
		(m_emptyLine ? "" : " "), tok);
	m_emptyLine = false;
}

////////////////////////////////////////////////////////////////////////////////
ParserErrorCtx::ParserErrorCtx(TokParser& parser, const char* ctxString)
	: m_parser(parser)
	, m_ctxString(ctxString)
	, m_errorString(nullptr)
	, m_ignored(false)
	, m_error(false)
	, m_errorLine(0)
{
}

bool ParserErrorCtx::Error(const char* errString)
{
	m_errorLine = m_parser.CurrentLine();
	m_errorString = errString;
	m_error = true;
	return false;
}
	
void ParserErrorCtx::ErrorString(const char* errString)
{
	m_errorString = errString;
}

ParserErrorCtx::~ParserErrorCtx()
{
	if(!m_ignored && (m_error || m_parser.Error())) {
		std::cerr << "Error:" <<
			m_ctxString << 
			":" << 
			(m_errorLine ? m_errorLine : m_parser.CurrentLine()) <<
			": ";
		if(m_errorString)
			std::cerr << m_errorString;
		else
			std::cerr << "error";
		std::cerr << std::endl;
	}
}

}

