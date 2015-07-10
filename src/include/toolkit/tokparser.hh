#pragma once
#ifndef INCLUDED_toolkit_tokparser_HH
#define INCLUDED_toolkit_tokparser_HH

#include <cstdio>
#include <type_traits>
#include  "dynary.hh"

namespace lptk
{

////////////////////////////////////////////////////////////////////////////////
enum TokenType {
	T_NONE = -1,
	T_Start = 255,
	T_NUMERIC,
	T_BOOLEAN,
	T_STRING,
	T_SYMBOL,
	T_TOK,
};

////////////////////////////////////////////////////////////////////////////////
class Tokenizer
{
public:
	Tokenizer(const char* data, size_t size) : m_data(data), m_size(size), m_pos(0), m_curLine(1) {}

	void Reset() { m_pos = 0; m_curLine = 1; }
	bool Next(char* tok, int &tokType, size_t maxLen);

	size_t CurrentLine() const { return m_curLine; }
private:
	const char* m_data;
	size_t m_size;
	size_t m_pos;
	size_t m_curLine;
};

////////////////////////////////////////////////////////////////////////////////
class TokFlagDef
{
public:
	TokFlagDef(const char* name, int flag) : m_name(name), m_flag(flag) {}
	const char* m_name;
	int m_flag;
};

////////////////////////////////////////////////////////////////////////////////
class TokParser
{
public:
    TokParser();
	TokParser(const char* data, size_t size);
	operator bool() const { return Ok(); }
	bool Ok() const ;
	bool Error() const ;

	int GetString(char* buffer, int maxLen);
	int GetSymbol(char* buffer, int maxLen);
	int GetInt();
	unsigned int GetUInt();
	float GetFloat();
	double GetDouble();
	bool GetBool();
	int GetFlag(const TokFlagDef* flagdef, int flagdefCount);
	int GetEnum(const TokFlagDef* flagdef, int flagdefCount);
	bool ExpectTok(const char* str); 
	bool IsTok(const char* str); 
	bool IsTokType(int tokType);
    bool Skip();
	bool SkipBlock();

	size_t CurrentLine() const { return m_tokenizer.CurrentLine(); }

	// read either single values or values in the form of { v0 v1 v2 ...}
	DynAry<float> GetFloatStream();
	DynAry<double> GetDoubleStream();
	DynAry<bool> GetBoolStream();
	DynAry<int> GetIntStream();
	DynAry<unsigned int> GetUIntStream();
private:
	void Consume();

	Tokenizer m_tokenizer;
	char m_token[256];
	int m_tokType;
	bool m_error;
	bool m_eof;
};

////////////////////////////////////////////////////////////////////////////////
class ParserErrorCtx
{
	TokParser& m_parser;
	const char* m_ctxString;
	const char* m_errorString;
	bool m_ignored;			// call to ignore normal parsing errors (when you want to return false for
							// a non-parsing/interpreting reason)
	bool m_error;			// indicates interpretation error (not a parsing error)
	size_t m_errorLine;
public:
	ParserErrorCtx(TokParser& parser, const char* ctxString);
	bool Error(const char* errString = nullptr);
	void ErrorString(const char* errString);
	void Ignore() { m_ignored = true; }
	~ParserErrorCtx();
private:
	ParserErrorCtx(const ParserErrorCtx&) DELETED;
	ParserErrorCtx operator=(const ParserErrorCtx&) DELETED;
};

////////////////////////////////////////////////////////////////////////////////
class TokWriter
{
public:
	TokWriter(const char* filename);
	~TokWriter();
	operator bool() const { return Ok(); }
	bool Ok() const ;
	bool Error() const;

	TokWriter(const TokWriter&) DELETED ;
	TokWriter& operator=(const TokWriter&) DELETED ;

	void Comment(const char* str);
	void String(const char* str);
	void Symbol(const char* str);
	void Int(int value);
	void UInt(unsigned int value);
	void Float(float value);
	void Double(double value);
	void Bool(bool value);
	void Nl();
	void Flag(const TokFlagDef* flags, int flagsCount, int value);
	void Token(const char* str);
private:
	FILE* m_fp;
	bool m_error;
	bool m_emptyLine;
} ;

}

#endif

