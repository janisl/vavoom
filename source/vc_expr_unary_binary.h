
//==========================================================================
//
//	VUnary
//
//==========================================================================

class VUnary : public VExpression
{
public:
	enum EUnaryOp
	{
		Plus,
		Minus,
		Not,
		BitInvert,
		TakeAddress,
	};
	EUnaryOp		Oper;
	VExpression*	op;

	VUnary(EUnaryOp, VExpression*, const TLocation&);
	~VUnary();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	void EmitBranchable(VEmitContext&, VLabel, bool);
};

//==========================================================================
//
//	VUnaryMutator
//
//==========================================================================

class VUnaryMutator : public VExpression
{
public:
	enum EIncDec
	{
		PreInc,
		PreDec,
		PostInc,
		PostDec,
		Inc,
		Dec,
	};
	EIncDec			Oper;
	VExpression*	op;

	VUnaryMutator(EIncDec, VExpression*, const TLocation&);
	~VUnaryMutator();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	bool AddDropResult();
};

//==========================================================================
//
//	VBinary
//
//==========================================================================

class VBinary : public VExpression
{
public:
	enum EBinOp
	{
		Add,
		Subtract,
		Multiply,
		Divide,
		Modulus,
		LShift,
		RShift,
		And,
		XOr,
		Or,
		Equals,
		NotEquals,
		Less,
		LessEquals,
		Greater,
		GreaterEquals,
	};
	EBinOp			Oper;
	VExpression*	op1;
	VExpression*	op2;

	VBinary(EBinOp, VExpression*, VExpression*, const TLocation&);
	~VBinary();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
};

//==========================================================================
//
//	VBinaryLogical
//
//==========================================================================

class VBinaryLogical : public VExpression
{
public:
	enum ELogOp
	{
		And,
		Or,
	};
	ELogOp			Oper;
	VExpression*	op1;
	VExpression*	op2;

	VBinaryLogical(ELogOp, VExpression*, VExpression*, const TLocation&);
	~VBinaryLogical();
	VExpression* DoResolve(VEmitContext&);
	void Emit(VEmitContext&);
	void EmitBranchable(VEmitContext&, VLabel, bool);
};
