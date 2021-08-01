/* Implementation of Recursive-Descent Parser
 * parse.cpp
 * Programming Assignment 3
*/

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <string.h>
#include "lex.cpp"
#include "val.h"
#include "parseRun.h"

using namespace std;

bool checkVal = false;
bool allowPrint = false;
bool allowIf = true;

int main(int argc, char* argv[]) {
    if (argc == 2) {
        fstream file(argv[1]);
        int lineNumber = 1;

        if (file.is_open()) {
            bool errorsFound = Prog(file, lineNumber);
            cout << endl;
            if (errorsFound) {
                
				cout << "Successful Interpretation" << endl;
            }
            else
            {
			
                cout << "Unsuccessful Interpretation" << endl;
                cout << endl;
                cout << "Number of Syntax Errors: " << error_count << endl;
            }
        }
        
        
        else {
            cout << "CANNOT OPEN FILE " << argv[1] << endl;
            return 0;
        }
    }
    else {
        cout << (argc > 2 ? "ONLY ONE FILE NAME ALLOWED" : "NO FILE NAME GIVEN") << endl;
        return 0;
    }
    return 0;
}

//Program is: Prog := begin StmtList end
bool Prog(istream& in, int& line)
{
	bool sl = false;
	LexItem tok = Parser::GetNextToken(in, line);
	// cout << "in Prog" << endl;
	
	if (tok.GetToken() == BEGIN) {
		sl = StmtList(in, line);
		if( !sl  )
			ParseError(line, "No statements in program");
		if( error_count > 0 )
			return false;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	
	if (tok.GetToken() == END)
		return true;
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else
		return false;
}

// StmtList is a Stmt followed by semicolon followed by a StmtList
 bool StmtList(istream& in, int& line) {
 	// cout << "in StmtList" << endl;
	bool status = Stmt(in, line);
		
	if( !status )
		return false;
	LexItem tok = Parser::GetNextToken(in, line);
	
	if( tok == SCOMA ) {	
		status = StmtList(in, line);
		
	}
	else if (tok == ERR) {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else if (tok == END) {
		Parser::PushBackToken(tok);
		return true;
	}
	else {
		
		ParseError(line, "Missing semicolon");
		return false;
	}
	
	return status;
}


//Stmt is either a PrintStmt, IfStmt, or an AssigStmt
bool Stmt(istream& in, int& line) {
	bool status;
//	cout << "in Stmt" << endl;
	LexItem t = Parser::GetNextToken(in, line);
	
	switch( t.GetToken() ) {

	case PRINT:
		status = PrintStmt(in, line);
		// cout << "status: " << (status? true:false) <<endl;
		break;

	case IF:
		Parser::PushBackToken(t);
		status = IfStmt(in, line);
		break;

	case IDENT:
        Parser::PushBackToken(t);
		status = AssignStmt(in, line);
		break;

	case END:
		Parser::PushBackToken(t);
		return true;
	case ERR:
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << t.GetLexeme() << ")" << endl;
		return false;
	case DONE:
		return false;

	default:
		ParseError(line, "Invalid statement");
		return false;
	}

	return status;
}

//PrintStmt:= print ExpreList 


bool PrintStmt(istream& in, int& line){

	ValQue = new queue<Value>;
	
	allowPrint = true;
	bool ex = ExprList(in,line);
    allowPrint = false;
	
	
	if(!ex){
		
		ParseError(line,"Missing expression after print");
		
		while(!(* ValQue).empty()){
			
			ValQue->pop();
		}
		
		delete ValQue;
		return false;
	}
	
	LexItem t = Parser :: GetNextToken(in,line);
	
	if(t.GetToken()==SCOMA)
	{
		while(!(*ValQue).empty())
		{
			
			Value nextVal =(*ValQue).front();
			cout << nextVal;
			ValQue->pop();
		}
		cout << endl;
	}
	
	Parser :: PushBackToken(t);
	return ex;

} 

//IfStmt:= if (Expr) then Stmt
bool IfStmt(istream& in, int& line) {
	// cout << "in ifStmt" << endl;
	LexItem item = Parser::GetNextToken(in, line);
	
	bool noerrors = true;
    checkVal = true;
	
	
    if (item.GetToken() != IF) {
        ParseError(line, "Missing If Statement Expression");
        noerrors = false;
    }

    item = Parser::GetNextToken(in, line);
    
    if (item.GetToken() != LPAREN) {
        ParseError(line, "Missing Left Parenthesis");
        noerrors = false;
    }
    Value expressionValue;
    bool flag = Expr(in, line, expressionValue);
	
    if (!flag) {
    // ParseError(line, "Invalid if statement Expression");

        noerrors = false;
    }
//    string s = item.GetLexeme();
//    cout << defVar.find(s) -> second;
	
	
    if (!expressionValue.IsInt()) {
        ParseError(line, "Run-Time: Non Integer If Statement Expression");
        noerrors = false;
    }
    else {
        if (expressionValue.GetInt() == 0)
            allowIf = false;

    }

    item = Parser::GetNextToken(in, line);
    if (item.GetToken() != RPAREN) {
        ParseError(line, "Missing Right Parenthesis");
        noerrors = false;
    }

    item = Parser::GetNextToken(in, line);
    if (item.GetToken() != THEN) {
        ParseError(line, "Missing THEN");
        noerrors = false;
    }

   
    if (noerrors == false)
        allowPrint = false;

    if (!Stmt(in, line)) {
        ParseError(line, "Invalid If Statement Expression");
        noerrors = false;
    }

    
    if (noerrors == false)
        allowPrint = false;

    return noerrors;
}

//Var:= ident
bool Var(istream& in, int& line, LexItem &tok)
{
//	cout << "in Var" << endl;
	LexItem item = Parser::GetNextToken(in, line);
    tok = item;
    
    if (item.GetToken() == IDENT) {   	
        if (defVar.find(item.GetLexeme()) != defVar.end()) {	
            defVar[item.GetLexeme()] = false;
            checkVal = false;
        }
        else {       	
            if (checkVal) {
                ParseError(line, "Undefined variable used in expression");
                checkVal = false;
                allowPrint = false;
                return false;
            }
            defVar[item.GetLexeme()] = true;
        }
    }
    else {
        ParseError(line, "Invalid Identifier Expression");
        checkVal = false;
        return false;
    }
    return true;
}

//AssignStmt:= Var = Expr
bool AssignStmt(istream& in, int& line) {
//	cout << "in Assign" << endl;
	const int currentLine = line;
	bool noerrors = true;
    LexItem tok;
    string identifier = "";
    
    
    
    if (!Var(in, line, tok)) {
		ParseError(currentLine, "Invalid Assignment Statement Identifier");
        noerrors = false;
    }
    else 
        identifier = tok.GetLexeme();
       
    LexItem item = Parser::GetNextToken(in, line);
    if (item.GetToken() != EQ) {
        ParseError(currentLine, "Missing Assignment Operator =");
        noerrors = false;
    }

   
    Value val;
    if (!Expr(in, line, val)) {
        ParseError(currentLine, "Invalid Assignment Statement Expression");
        noerrors = false;
    }
	
    if (noerrors && identifier != "" && allowIf) {
        
        
		
        if (symbolTable.find(identifier) == symbolTable.end()) {
            symbolTable.insert(make_pair(identifier, val));
            
        }
        
        
        else {
            Value& actualType = symbolTable.find(identifier)->second;

           
            if (actualType.IsInt() && val.IsInt() || actualType.IsReal() && val.IsReal()
                || actualType.IsStr() && val.IsStr())
                symbolTable[identifier] = val;
                
            else if (actualType.IsInt() && val.IsReal())
                symbolTable[identifier] = Value((int) val.GetReal());
                
            else if (actualType.IsReal() && val.IsInt())
            	symbolTable[identifier] = Value((float) val.GetInt());
            
			else {
                ParseError(currentLine, "Run-Time: Illegal assignment Operation");
                noerrors = false;
            }
        }
    }    
	
    if (noerrors == false)
        allowPrint = false;
    allowIf = true;
    return noerrors; 
}

//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
//	cout << "in ExprList" << endl;
	bool status = false;
	
	Value Val;
	status = Expr(in, line, Val);
    if (allowPrint == true) {
        (*ValQue).push(Val);
            
    }
	if(!status){
		ParseError(line, "Missing Expression");
		return false;
	}
	
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == COMA) {
		status = ExprList(in, line);
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}

//Expr:= Term {(+|-) Term}
bool Expr(istream& in, int& line, Value & retVal) {
//	cout << "in Expr" << endl;
	
	bool noerrors = true;
    Value accumulator;
    Value current;
    int operation = -1;

    while (true) {
        if (!Term(in, line, current)) {
            //ParseError(line, "Invalid Term Expression");
            return false;
        }
        else {
            
            if (accumulator.IsErr())
                accumulator = current;
            else if (operation == 0) {
                
                
				if (accumulator.IsStr() || current.IsStr()) {
                    ParseError(line, "Run-Time:Error Illegal Mixed Type operation");
                    noerrors = false;
                }
                else
                    accumulator = accumulator + current;
            }
            else if (operation == 1) {
                if (accumulator.IsStr() || current.IsStr()) {
                    ParseError(line, "Run-Time: Invalid Arithmetic Operation");
                    noerrors = false;
                }
                else
                    accumulator = accumulator - current;
            }
        }

        LexItem item = Parser::GetNextToken(in, line);
        if (item.GetToken() == ERR)
            noerrors = false;
        else if (item.GetToken() == PLUS)
            operation = 0;
        else if (item.GetToken() == MINUS)
            operation = 1;
        else {
            Parser::PushBackToken(item);
            retVal = accumulator;
			return noerrors;
        }
    }
    return false;  
}

//Term:= Factor {(*|/) Factor}
bool Term(istream& in, int& line, Value & retVal) {
	
//	cout << "in term" << endl;
	
	bool noerrors = true;
    Value accumulator;
    Value current;
    int operation = -1;
    while (true) {
        if (!Factor(in, line, current)) {
            // ParseError(line, "Invalid Term Expression");
            return false;
        }
        else {
            
            if (accumulator.IsErr())
                accumulator = current;
            else if (operation == 0) {
                
                if (accumulator.IsStr() || current.IsStr()) {
                    ParseError(line, "Run-Time: Invalid String Operation");
                    noerrors = false;
                }
                else
                    accumulator = accumulator * current;
            }
            else if (operation == 1) {
                if (accumulator.IsStr() || current.IsStr()) {
                    ParseError(line, "Run-Time: Invalid Arithmetic Operation");
                    noerrors = false;
                }
                else{
					
					accumulator = accumulator / current;
						
				}
            }
        }

        LexItem item = Parser::GetNextToken(in, line);
        if (item.GetToken() == ERR)
            noerrors = false;
        else if (item.GetToken() == MULT)
            operation = 0;
        else if (item.GetToken() == DIV)
            operation = 1;
        else {
            Parser::PushBackToken(item);
			retVal = accumulator;
            return noerrors;
        }
    }
    return false;  
}

//Factor := ident | iconst | rconst | sconst | (Expr)
bool Factor(istream& in, int& line, Value & retVal) {
//	cout << "in Factor" << endl;
	LexItem item = Parser::GetNextToken(in, line);
	Token tok = item.GetToken();
	
	
	const int lineNumber = item.GetLinenum();
	if( tok == IDENT ) {
		
		Parser::PushBackToken(item);
		
        if (!Var(in, line, item))
        {
        	return false;
		}
        retVal = symbolTable[item.GetLexeme()];
        //  if (allowPrint == true) {
            
        //     (*ValQue).push(retVal);
        // }
        
        return true;
		
	}
	else if( tok == ICONST ) {
		checkVal = false;
        retVal = Value(std::stoi(item.GetLexeme()));
        
        // if (allowPrint == true) {
        //     (*ValQue).push(retVal);
        // }
        return true;
		
	}
	else if( tok == SCONST ) {
		
		checkVal = false;
        retVal = Value(item.GetLexeme());
        // if (allowPrint == true) {
        //     (*ValQue).push(retVal);
            
        // }
        return true;
	}
	else if( tok == RCONST ) {
	
		checkVal = false;
        retVal = Value(std::stof(item.GetLexeme()));
        // if (allowPrint == true) {
        //     (*ValQue).push(retVal);
        // }
        return true;
	}
	else if( tok == LPAREN ) {
		bool ex = Expr(in, line,retVal);
		if( !ex ) {
			ParseError(line, "Missing expression after (");
			return false;
		}
		if( Parser::GetNextToken(in, line) == RPAREN )
			return ex;

		ParseError(line, "Missing ) after expression");
		return false;
	}
	else if(item.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << item.GetLexeme() << ")" << endl;
		return false;
	}

	ParseError(line, "Unrecognized input");
	return false;
}



