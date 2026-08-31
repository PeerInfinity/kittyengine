#pragma once

class Console
{
public:
	Console();
	~Console();

	void					Go();
	void					Out(char *format, ...);
	void					OutPlain(char *theText);
#ifdef _DEBUG
	void					Debug(char *format, ...);
#else
	inline void				Debug(char *format, ...) {}
#endif
	inline void				On() {mOn=true;}
	inline void				Off() {mOn=false;}

	void					Alert(char *format, ...);
	void					XAlert(char *format, ...);



public:
	bool					mOn;

public:
	inline void				Indent() {mIndent+=4;}
	inline void				Unindent() {mIndent-=4;if (mIndent<0) mIndent=0;}
	inline void				ResetIndent() {mIndent=0;}
	void					operator++(int) {Indent();}
	void					operator--(int) {Unindent();}
	void					operator++() {Indent();}
	void					operator--() {Unindent();}

private:
	int						mIndent;
	String					mIndentString;

};

extern Console gConsole;
// void Echo(char *format, ...);


//
// Tracer is like the console except it's not for detecting actual crashes, etc.
// It's for detecting program logic flow.  I put it in here during Solomon Dark
// to figure out why Jason's play button doesn't work.
//
// Remember, tracer is SHORT strings... limit MAX_PATH chars!
//


class IOBuffer;
class Tracer
{
public:
	Tracer();
	~Tracer();
	void					Out(char *format, ...);

public:
	IOBuffer*				mBuffer;
	String					mFilename;
};

