#include <fstream>
#include <iostream>
#include <string> 
#include <cstdlib>
#include <sstream>

using namespace std;

string find_translate(string src);
void leitura(); 
int identifica(string leitura); // OK
int ler_immed(string valor); // (OK)
int toInt(string valor); // (OK)
void separa(string *in, string *out1, string *out2, string *out3); // (OK)
bool isReg(string test); // (OK)
bool isImmed(string test); // (OK)
int searchInstruction(string order[], string vals[], string name); //OK 
int add_in(int val_base, int sfht, string ref); //OK
int searchName(string name); //OK
bool is_aLabel(string test); //OK
string specialReturn(string test); // OK
int push_pop(int base, string s_regs); // OK
void marca_label(); //OK
int sp_branch(string branch, string label); // OK
int label_line(string label); // OK
string rm_chave(string test); // OK
int ldr_str(string inst, string ordr[]); // Em desenvolvimento
int st_ldMultiple(int base, string s_regs, string rgr);
//stmia base = 49152
//ldmia base = 51200
int BX_BL(string name, string arg);

int main (){
	/*
	string inst = "piromba";
	string p[3] = {"x" ,"x" , "x"};
	separa(&inst, &p[0], &p[1], &p[2]); 
	cout << p[1] << endl;
	*/
	
	//cout << BX_BL("bl", "pitanga") << endl;
	
	marca_label();
	leitura();
	
	system("pause");
}

void leitura(){ // Parte 1 do automato, *ler*

	//Tabela de entrada
	ifstream in_table;
	in_table.open("in.txt"); // ==================================== ARQUIVO DE ENTRADA ===============================
	/*=================*/
	//Tabela de saida
	ofstream out_file;
	out_file.open("compiled.txt"); // =============================== ARQUIVO DE SAIDA =================================
	/*=================*/
	
	/* Leitura de dados (in) */ 
	string leitura;
	/********************/
	std::getline(in_table, leitura); // Pegando toda uma linha de codigo
	int out_decode ; // Saida decodificada
	int index = 0; // marcador de linha lida
	int g32 = 0;
	
	out_file << index << " : "; //Escreve o primeiro indice lido
	while(leitura != "b ."){

		if( (leitura != " ") && (leitura != "\n") && (leitura != "\0") && !is_aLabel(leitura)){ // Nao pega fim de texto, espaco ou quebra ded linha
			if(g32 % 2  == 0 && g32 !=0){
				out_file << endl;
				out_file << g32*2 << " : "; //Escreve o indice lido
			}
			
			cout << leitura << endl;
			out_decode = identifica(leitura);
			//cout << leitura << " decoded: " << hex <<  out_decode << endl;
			if(out_decode == -1 || out_decode < 0){
				cout << "erro na linha: "<< index + 1<< " instrucao nao definida!"<< endl;
				return;
			}
			if(out_decode < 256){
				out_file  << "00";
			}
			if(out_decode < 4096){
				out_file  << "0";
			}
			out_file << hex << out_decode; // escreve o valor em hexa correspondente
			g32++;
		}
		std::getline(in_table, leitura);
		index ++;
	}
	in_table.close();
	out_file.close();
	cout << "Traducao completa!" << endl;
	return;
}

int identifica(string leitura){ //Identifica qual tipo de operando
	string lt1 = leitura;
	string opm[3];
	separa(&lt1, &opm[0], &opm[1], &opm[2]); //Separada leitura
	string order[3];
	opm[0] = rm_chave(opm[0]);
	opm[1] = rm_chave(opm[1]);
	opm[2] = rm_chave(opm[2]);
	//--------------------------Casos especiais----------------------------//
	int branchs = sp_branch(lt1, opm[0]);
	if(branchs == -1){
		return -1;
	}
	if(branchs > 0){
		return branchs;
	}
	
	int bx_special = BX_BL(lt1, opm[0]);
	if(bx_special > 0){
		return bx_special;
	}
	
	int ldrstr = ldr_str(lt1, opm);
	if(ldrstr != -404){
		return ldrstr;
	}
	
	if(lt1 == "push"){
		return push_pop(46080, opm[0]);
	}
	if(lt1 == "pop"){
		return push_pop(48128, opm[0]);
	}
	if(lt1 == "ldmia"){
		return st_ldMultiple(51200, opm[1], opm[0]);
	}
	if(lt1 == "stmia"){
		return st_ldMultiple(49152, opm[1], opm[0]);
	}
	//--------------------------------------------------------------------//
	for(int i=0; i<3; i++){ // Identificando quantidade de registradores e imediatos
		if(isReg(opm[i])){ // e registrador?
			order[i] = "reg";
		}else if(isImmed(opm[i])){ // senao, e imediato?
			order[i] = "ime";
		}else{//se nenhum, sai do estado
			order[i] = 'x';
		}
	}
	//cout << lt1 << opm[0] << opm[1] << opm[2] << endl;
	 int val_base = searchInstruction(order, opm, lt1);
	 return val_base;
}
/***********************************************************************/ 
/***********************************************************************/
int searchInstruction(string order[], string vals[], string name){
	ifstream com_table;
	com_table.open("table.txt");
	// variaveis 
	string lt_tmp; // leitor temporario e auxiliar
	string id_pointer; // leitor iterador
	/********************/
	string tester[3];
	com_table >> id_pointer; // Le o nome
	
	com_table >> tester[0];
	com_table >> tester[1];
	com_table >> tester[2];
	
	int x1,x2,x3;
	int base_vl = 5;
	
	com_table >> lt_tmp;
	x1 = toInt(lt_tmp);
	com_table >> lt_tmp;
	x2 = toInt(lt_tmp);
	com_table >> lt_tmp;
	x3 = toInt(lt_tmp);
	/********************/
	while(id_pointer != ".endf"){ // Procura instrucao equivalente
	
		if(id_pointer == name){ // procurando instrucao parecida, parte 1
			if(order[0] == tester[0]){
			//	cout << "o1" << endl;
				if(order[1] == tester[1]){
				//	cout << "o2" << endl;
					if(order[2] == tester[2]){
					//	cout << "o3" << endl;
						com_table >> id_pointer;
						base_vl = toInt(id_pointer); // Pega o valor base;
						int ret;
						ret = add_in(base_vl,x1, vals[0]);
						ret = add_in(ret, x2, vals[1]);
						ret = add_in(ret, x3, vals[2]);
						return ret;
					}
				}
			}
		}
		com_table >> lt_tmp;
		com_table >> id_pointer;
		
		if(id_pointer != ".endf"){
			com_table >> tester[0];
			com_table >> tester[1];
			com_table >> tester[2];
			
			com_table >> lt_tmp;
			x1 = toInt(lt_tmp);
			com_table >> lt_tmp;
			x2 = toInt(lt_tmp);
			com_table >> lt_tmp;
			x3 = toInt(lt_tmp);
		}
	}
	com_table.close();
	return -1;
}
/***********************************************************************/
int ler_immed(string valor){ // retira o # do imediato e retorna o inteiro (OK)
	char imediate[10];
	if(valor[0] == '#'){
		for(int i = 1; i < valor.length(); i++){
			imediate[i-1] = valor[i];
		}
		int retu = atoi(imediate);
		return retu;
		}else{
			return -1; //cod erro
		}
		
}
/**********************************************************************/
int add_in(int val_base, int sfht, string ref){ // Faz a operacao de organizar o valor de saida final
	ref = specialReturn(ref);
	if(isImmed(ref)){
	//	cout << "isImediate!" << endl;
		int iref = ler_immed(ref);
		iref = iref<<sfht;
		return val_base + iref;
	}
	if(isReg(ref)){
	//	cout << "isReg!" << endl;
		int iref = searchName(ref);
		iref = iref<<sfht;
		return val_base + iref;
	}
	if(ref == "x"){
	//	cout << "Irrelevant" << endl;
		return val_base;
	}
	return 0;
}
/***********************************************************************/
int searchName(string name){ //-----------------------------------Procura na tabela de nomes-------------------
	ifstream com_table;
	com_table.open("reg_name.txt");
	string ltr;
	com_table >> ltr;
	while(ltr != ".endf"){
		if(ltr == name){
			com_table >> ltr;
			return toInt(ltr);
		}
		com_table >> ltr;
	}
	com_table.close();
	return -1;
}
/**********************************************************************/
int toInt(string valor){ // Converte para int (OK)
	char imediate[10];
	for(int i = 0; i < valor.length(); i++){
		imediate[i] = valor[i];
	}
	int retu = atoi(imediate);
	return retu;
}
/**********************************************************************/
bool isReg(string test){ // Retorna se um string e sobre um registrador ou nao (OK)
	if(test[0] == 'r' || test == "pc" || test == "lr" || test == "sp"){
		return true;
	}
	return false;
}
/**********************************************************************/ 
bool isImmed(string test){ // Retorna se um string e sobre um imediato ou nao (OK)
	if(test[0] == '#'){
		return true;
	}
	return false;
}
/**********************************************************************/ 
void separa(string *in, string *out1, string *out2, string *out3){ // Separa (OK)
	string tmp;
	string modifi = *in;
	int i = 0;
	int before;
	
	before = 0;
	while(modifi[i] != 32 && modifi[i] != '\0'){
		i++;
	}
	tmp = modifi.substr(before, i);
	*in = tmp;
	/*******OUT1**************/
	if(modifi[i] != '\n' && modifi[i] != '\0'){
		
		before = i;
		i++;
	while(modifi[i] != 32 && modifi[i] != '\0'){
		i++;
	}
	tmp = modifi.substr(before+1, i-before-1);
	*out1 = tmp;
	}else{
		*out1 = "x";
		*out2 = "x";
		*out3 = "x";
		return;
	}
	/*******OUT2**************/
	if(modifi[i] != '\n' && modifi[i] != '\0'){
		before = i;
		i++;
	while(modifi[i] != 32 && modifi[i] != '\0'){
		i++;
	}
	tmp = modifi.substr(before+1, i-before-1);
	*out2 = tmp;
	}else{
		*out2 = "x";
		*out3 = "x";
		return;
	}
	/*******OUT3**************/
	if(modifi[i] != '\n' && modifi[i] != '\0'){
		before = i;
		i++;
	while(modifi[i] != 32 && modifi[i] != '\n' && modifi[i] != '\0'){
		i++;
	}
	tmp = modifi.substr(before+1);
	*out3 = tmp;
	}else{
		*out3 = "x";
		return;
	}
	/**/
	return;
}
/**********************************************************************/
bool is_aLabel(string test){//Verifica um label O(n)
	
	if(test[test.size()-1] == ':'){
		return true;
	}

}
/**********************************************************************/

int push_pop(int base, string s_regs){ // funcao especifica para push e pop
	s_regs = specialReturn(s_regs);
	int i = 0;
	int befo = i;
	int pos = 0; //x1
	int npos = 0; //x2
	while(s_regs[i] != '-' && s_regs[i] != '\0' && s_regs[i] != '\n'){
		i++;
	}
	pos = searchName(s_regs.substr(befo, i));
	befo = i+1;
	
	if(s_regs[i] == '-'){
		i++;
		while(s_regs[i] != '-' && s_regs[i] != '\0' && s_regs[i] != '\n'){
		i++;
		}
		npos = searchName(s_regs.substr(befo, i-1));
	}else{
		int shftd = 1 << pos;
		return base + shftd;
	}

	if(npos < 0 || pos < 0){
		return -1;
	}
	if(pos > npos){
		int aux = npos;
		npos = pos;
		pos = aux;
	}
	
	for(int ix = pos; ix <= npos; ix++){
		int shftd = 1 << ix;
		base += shftd;
	}
	return base;
	
}
/**********************************************************************/
string specialReturn(string test){ // Se for da formula {xxx} retorna o conteudo do colchete
	if(test[0] == '{' && test[test.size()-1] == '}'){
		return test.substr(1, test.size()-2);
	}
	return test;
}
/**********************************************************************/
void marca_label(){ // le os label e guarda-os na linha que aparecem para serem usados no branch
	ifstream in_file;
	in_file.open("in.txt"); // ============================================ ARQUIVO DE ENTRADA ================================================
	ofstream lbl_arq;
	lbl_arq.open("label_pos.txt");
	int index = 1;
	string lt_pointer;
	string aux_pointer;
	std::getline(in_file, aux_pointer);
	lt_pointer = aux_pointer.substr(0,aux_pointer.size()-1);

	while(aux_pointer != "b ." && aux_pointer != "b."){
		if(is_aLabel(aux_pointer)){
			lbl_arq << lt_pointer << '\n' << index << '\n';
		}
		index++;
		std::getline(in_file, aux_pointer);
		lt_pointer = aux_pointer.substr(0,aux_pointer.size()-1);
	}
	lbl_arq << ".endf" << endl << '\0';
	lbl_arq.close();
	in_file.close();
	return;
}
/**********************************************************************/
int sp_branch(string branch, string label){ // resolve operacao de branch
	ifstream branch_file;
	branch_file.open("branch_bases.txt");
	string pointer;
	branch_file >> pointer;
	while(pointer != ".endf"){
		if(pointer == branch){ // se encontrar o branch correspondente
			string bvalue;
			branch_file >> bvalue; // le o valor da base
			int labelLine = label_line(label);
			if(labelLine < 0){
				branch_file.close();
				return -1;
			}
			int b_calcl = toInt(bvalue) + labelLine;
			branch_file.close();
			return b_calcl;
		}
		branch_file >> pointer;
	}
	branch_file.close();
	return -404; //se nao encontrar ou nao branch
}
/**********************************************************************/
int label_line(string label){ // retorna o valor da linha de um label
	ifstream lbl_arq;
	lbl_arq.open("label_pos.txt");
	string pointer;
	lbl_arq >> pointer;
	while(pointer != ".endf"){
		if(pointer == label){
			string bvalue;
			lbl_arq >> bvalue;
			lbl_arq.close();
			return toInt(bvalue);
		}
		lbl_arq >> pointer;
	}
	lbl_arq.close();
	return -1;
}
/**********************************************************************/
string rm_chave(string test){ // remove chave
	if(test[0] == '['){
		test = test.substr(1, test.size());
	}
	if(test[test.size()-1] == ']'){
		return test.substr(0, test.size()-1);
	}
	return test;
}
/**********************************************************************/
int ldr_str(string inst, string ordr[]){ // Casos especiais do LDR
int base = 404;
	if(inst == "ldr"){
		if(ordr[1] == "sp" || ordr[1] == "sp,"){
			base = 38912;
			int sft = searchName(ordr[0]) << 8;
			base += sft;
			return base + (ler_immed(ordr[2]) * 4);
		}
		if(ordr[1] == "pc" || ordr[1] == "pc,"){
			base = 18432;
			int sft = searchName(ordr[0]) << 8;
			base += sft;
			return base + (ler_immed(ordr[2]) * 4);
		}
	}
	if(inst == "str"){
		if(ordr[1] == "sp" || ordr[1] == "sp,"){
			base = 36864;
			int sft = searchName(ordr[0]) << 8;
			base += sft;
			return base + (ler_immed(ordr[2]) * 4);
		}
	}
	if(inst == "add"){
		if(ordr[1] == "sp" || ordr[1] == "sp,"){
			base = 40960;
			int sft = searchName(ordr[0]) << 8;
			base += sft;
			return base + (ler_immed(ordr[2]) * 4);
		}
	}
	return -404;
}
/**********************************************************************/
int st_ldMultiple(int base, string s_regs, string rgr){ // funcao especifica para stmia e ldmia
	s_regs = specialReturn(s_regs);
	int i = 0;
	int befo = i;
	int pos = 0; //x1
	int npos = 0; //x2
	
	int reg_val = searchName(rgr);
	reg_val = reg_val << 8;
	base += reg_val;
	while(s_regs[i] != '-' && s_regs[i] != '\0' && s_regs[i] != '\n'){
		i++;
	}
	pos = searchName(s_regs.substr(befo, i));
	befo = i+1;
	
	if(s_regs[i] == '-'){
		i++;
		while(s_regs[i] != '-' && s_regs[i] != '\0' && s_regs[i] != '\n'){
		i++;
		}
		npos = searchName(s_regs.substr(befo, i-1));
	}else{
		int shftd = 1 << pos;
		return base + shftd;
	}

	if(npos < 0 || pos < 0){
		return -1;
	}
	if(pos > npos){
		int aux = npos;
		npos = pos;
		pos = aux;
	}
	
	for(int ix = pos; ix <= npos; ix++){
		int shftd = 1 << ix;
		base += shftd;
	}
	return base;
	
}
/****************************************************************/
int BX_BL(string name, string arg){
	if(name == "bl"){
		if(isReg(arg)){
			int tst = toInt(arg.substr(1));
			if(tst < 0){
				return -1;
			}
			return 63488 + tst;
		}
		int tst = label_line(arg);
			if(tst < 0){
				return -1;
			}
		return 63488 + tst;
	}
	
	if(name == "blx"){
		int tst;
		if(isReg(arg)){
			tst = toInt(arg.substr(1));
			if(tst < 0){
				return -1;
			}
			tst = tst << 3;
		return 59392 + tst;
		}
			tst = label_line(arg);
			if(tst < 0){
				return -1;
			}
		return 59392 + tst;
	}
	
	if(name == "bx"){
		int tmp = toInt(arg);
		if(tmp > 15){
			return -1;
		}
		tmp = tmp << 3;
		return 18176 + tmp;
	}
	return -1;
}



