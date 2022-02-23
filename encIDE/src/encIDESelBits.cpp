#include "encIDESelBits.h"
#include "encIDEApp.h"

#include <wx/msgdlg.h>
#include <wx/string.h>
#include <wx/intl.h>
#include <wx/display.h>
#include <wx/font.h>
#include <wx/colour.h>

#include <algorithm>
#include <memory>

const long encIDESelBits::ID_CHOICE1 = wxNewId();
const long encIDESelBits::ID_STATICTEXT1 = wxNewId();

int getInstType(std::string inst);
//std::string CreateDynamicButtons(encIDESelBits *thisWindow, std::string inst);
void RemoveAllDynamicButtons(encIDESelBits *thisWindow, std::string inst);

const long encIDESelBits::ID_BUTTON1 = wxNewId();
const long encIDESelBits::ID_BUTTON3 = wxNewId();
const long encIDESelBits::ID_STATICTEXT2 = wxNewId();

BEGIN_EVENT_TABLE(encIDESelBits,wxFrame)

END_EVENT_TABLE()

std::vector<std::string> opcodes_alli2{
"lui"
,"auipc"
,"jal"
,"jalr"
,"beq"
,"bne"
,"blt"
,"bge"
,"bltu"
,"bgeu"
,"lb"
,"lh"
,"lw"
,"lbu"
,"lhu"
,"sb"
,"sh"
,"sw"
,"addi"
,"slti"
,"sltiu"
,"xori"
,"ori"
,"andi"
,"slli"
,"srli"
,"srai"
,"add"
,"sub"
,"sll"
,"slt"
,"sltu"
,"xor_"
,"srl"
,"sra"
,"or_"
,"and_"
,"fence"
,"fence_i"
,"ecall"
,"ebreak"
,"csrrw"
,"csrrs"
,"csrrc"
,"csrrwi"
,"csrrsi"
,"csrrci"
,"lwu"
,"ld"
,"sd"
//,"slli"
//,"srli"
//,"srai"
,"addiw"
,"slliw"
,"srliw"
,"sraiw"
,"addw"
,"subw"
,"sllw"
,"srlw"
,"sraw"
,"mul"
,"mulh"
,"mulhsu"
,"mulhu"
,"div_"
,"divu"
,"rem"
,"remu"
,"mulw"
,"divw"
,"divuw"
,"remw"
,"remuw"
,"lr_w"
,"sc_w"
,"amoswap_w"
,"amoadd_w"
,"amoxor_w"
,"amoand_w"
,"amoor_w"
,"amomin_w"
,"amomax_w"
,"amominu_w"
,"amomaxu_w"
,"lr_d"
,"sc_d"
,"amoswap_d"
,"amoadd_d"
,"amoxor_d"
,"amoand_d"
,"amoor_d"
,"amomin_d"
,"amomax_d"
,"amominu_d"
,"amomaxu_d"
,"flw"
,"fsw"
,"fmadd_s"
,"fmsub_s"
,"fnmsub_s"
,"fnmadd_s"
,"fadd_s"
,"fsub_s"
,"fmul_s"
,"fdiv_s"
,"fsqrt_s"
,"fsgnj_s"
,"fsgnjn_s"
,"fsgnjx_s"
,"fmin_s"
,"fmax_s"
,"fcvt_w_s"
,"fcvt_wu_s"
,"fmv_x_w"
,"feq_s"
,"flt_s"
,"fle_s"
,"fclass_s"
,"fcvt_s_w"
,"fcvt_s_wu"
,"fmv_w_x"
," fcvt_l_s"
," fcvt_lu_s"
," fcvt_s_l"
," fcvt_s_lu"
,"fld"
,"fsd"
,"fmadd_d"
,"fmsub_d"
,"fnmsub_d"
,"fnmadd_d"
,"fadd_d"
,"fsub_d"
,"fmul_d"
,"fdiv_d"
,"fsqrt_d"
,"fsgnj_d"
,"fsgnjn_d"
,"fsgnjx_d"
,"fmin_d"
,"fmax_d"
,"fcvt_s_d"
,"fcvt_d_s"
,"feq_d"
,"flt_d"
,"fle_d"
,"fclass_d"
,"fcvt_w_d"
,"fcvt_wu_d"
,"fcvt_d_w"
,"fcvt_d_wu"
,"fcvt.l.d"
,"fcvt.lu.d"
,"fmv.x.d"
,"fcvt.d.l"
,"fcvt.d.lu"
,"fmv.d.x"
,"c_addi4spn"
,"c_fld"
//,"c_lq"
,"c_lw"
,"c_flw"
,"c_ld"
,"c_fsd"
//,"c_sq"
,"c_sw"
,"c_fsw"
,"c_sd"
,"c_nop"
,"c_addi"
,"c_jal"
,"c_addiw"
,"c_li"
,"c_addi16sp"
,"c_lui"
,"c_srli"
//,"c_srli64"
,"c_srai"
//,"c_srai64"
,"c_andi"
,"c_sub"
,"c_xor"
,"c_or"
,"c_and"
,"c_subw"
,"c_addw"
,"c_j"
,"c_beqz"
,"c_bnez"
,"c_slli"
//,"c_slli64"
,"c_fldsp"
//,"c_lqsp"
,"c_lwsp"
,"c_flwsp"
,"c_ldsp"
,"c_jr"
,"c_mv"
,"c_ebreak"
,"c_jalr"
,"c_add"
,"c_fsdsp"
//,"c_sqsp"
,"c_swsp"
,"c_fswsp"
,"c_sdsp"
};

std::vector<std::string> opcodes_rv32 {
"lui"
,"auipc"
,"addi"
,"slti"
,"sltiu"
,"xori"
,"ori"
,"andi"
,"slli"
,"srli"
,"srai"
,"add"
,"sub"
,"sll"
,"slt"
,"sltu"
,"xor"
,"srl"
,"sra"
,"or"
,"and"
,"fence"
,"fence.i"
,"csrrw"
,"csrrs"
,"csrrc"
,"csrrwi"
,"csrrsi"
,"csrrci"
,"ecall"
,"ebreak"
,"uret"
,"sret"
,"mret"
,"wfi"
,"sfence.vma"
,"lb"
,"lh"
,"lw"
,"lbu"
,"lhu"
,"sb"
,"sh"
,"sw"
,"jal"
,"jalr"
,"beq"
,"bne"
,"blt"
,"bge"
,"bltu"
,"bgeu"
,"mul"
,"mulh"
,"mulhsu"
,"mulhu"
,"div"
,"divu"
,"rem"
,"remu"
,"lr.w"
,"sc.w"
,"amoswap.w"
,"amoadd.w"
,"amoxor.w"
,"amoand.w"
,"amoor.w"
,"amomin.w"
,"amomax.w"
,"amominu.w"
,"amomaxu.w"
,"fmadd.s"
,"fmsub.s"
,"fnmsub.s"
,"fnmadd.s"
,"fadd.s"
,"fsub.s"
,"fmul.s"
,"fdiv.s"
,"fsqrt.s"
,"fsgnj.s"
,"fsgnjn.s"
,"fsgnjx.s"
,"fmin.s"
,"fmax.s"
,"fcvt.w.s"
,"fcvt.wu.s"
,"fmv.x.w"
,"feq.s"
,"flt.s"
,"fle.s"
,"fclass.s"
,"fcvt.s.w"
,"fcvt.s.wu"
,"fmv.w.x"
,"fmadd.d"
,"fmsub.d"
,"fnmsub.d"
,"fnmadd.d"
,"fadd.d"
,"fsub.d"
,"fmul.d"
,"fdiv.d"
,"fsqrt.d"
,"fsgnj.d"
,"fsgnjn.d"
,"fsgnjx.d"
,"fmin.d"
,"fmax.d"
,"fcvt.s.d"
,"fcvt.d.s"
,"feq.d"
,"flt.d"
,"fle.d"
,"fclass.d"
,"fcvt.w.d"
,"fcvt.wu.d"
,"fcvt.d.w"
,"fcvt.d.wu"
,"flw"
,"fsw"
,"fld"
,"fsd"
};

std::vector<std::string> Rtypes { // funct7 rs2 rs1 funct3 rd opcode
// rv32i
"add"
,"sub"
,"sll"
,"slt"
,"sltu"
,"xor_"
,"srl"
,"sra"
,"or_"
,"and_"

// rv64i
,"addw"
,"subw"
,"sllw"
,"srlw"
,"sraw"

// rv32m
,"mul"
,"mulh"
,"mulhsu"
,"mulhu"
,"div_"
,"divu"
,"rem"
,"remu"

// rv64m
,"mulw"
,"divw"
,"divuw"
,"remw"
,"remuw"

// rv32f
,"fsgnj_s"
,"fsgnjn_s"
,"fsgnjx_s"
,"fmin_s"
,"fmax_s"
,"fmv_x_w"
,"feq_s"
,"flt_s"
,"fle_s"
,"fclass_s"
,"fmv_w_x"

// rv32d
,"fsgnj_d"
,"fsgnjn_d"
,"fsgnjx_d"
,"fmin_d"
,"fmax_d"
,"feq_d"
,"flt_d"
,"fle_d"
,"fclass_d"

// rv64d
,"fmv_x_d"
,"fmv_d_x"

// rv32q
// not supported
,"fsgnj_q"
,"fsgnjn_q"
,"fsgnjx_q"
,"fmin_q"
,"fmax_q"
,"feq_q"
,"flt_q"
,"fle_q"
,"fclass_q"

};

// Actually R type
std::vector<std::string> Rshtypes { // funct7 rs2 rs1 funct3 rd opcode // funct7 shamt rs1 funct3 rd opcode
// rv32i
"slli"
,"srli"
,"srai"

// rv64i
,"slliw"
,"srliw"
,"sraiw"

// rv32q
// not supported
,"flq"

};

// Actually R type
std::vector<std::string> Raqtypes { // funct7 rs2 rs1 funct3 rd opcode // funct5 aq rl rs2 rs1 funct3 rd opcode
// rv32a
"lr_w"
,"sc_w"
,"amoswap_w"
,"amoadd_w"
,"amoxor_w"
,"amoand_w"
,"amoor_w"
,"amomin_w"
,"amomax_w"
,"amominu_w"
,"amomaxu_w"

// rv64a
,"lr_d"
,"sc_d"
,"amoswap_d"
,"amoadd_d"
,"amoxor_d"
,"amoand_d"
,"amoor_d"
,"amomin_d"
,"amomax_d"
,"amominu_d"
,"amomaxu_d"

};

// Actually R type
std::vector<std::string> Rrmtypes { // funct7 rs2 rs1 funct3 rd opcode // funct7 rs2 rs1 rm rd opcode
// rv32f
"fadd_s"
,"fsub_s"
,"fmul_s"
,"fdiv_s"
,"fsqrt_s"
,"fcvt_w_s"
,"fcvt_wu_s"
,"fcvt_s_w"
,"fcvt_s_wu"
,"fcvt_l_s"
,"fcvt_lu_s"
,"fcvt_s_l"
,"fcvt_s_lu"

// rv64f
," fcvt_l_s"
," fcvt_lu_s"
," fcvt_s_l"
," fcvt_s_lu"

// rv32d
,"fadd_d"
,"fsub_d"
,"fmul_d"
,"fdiv_d"
,"fsqrt_d"
,"fcvt_s_d"
,"fcvt_d_s"
,"fcvt_w_d"
,"fcvt_wu_d"
,"fcvt_d_w"
,"fcvt_d_wu"

// rv64d
,"fcvt_l_d"
,"fcvt_lu_d"
,"fcvt_d_l"
,"fcvt_d_lu"

// rv32q
// not supported
,"fadd_q"
,"fsub_q"
,"fmul_q"
,"fdiv_q"
,"fsqrt_q"
,"fcvt_s_q"
,"fcvt_q_s"
,"fcvt_d_q"
,"fcvt_q_d"
,"fcvt_w_q"
,"fcvt_wu_q"
,"fcvt_q_w"
,"fcvt_q_wu"

// rv64q
// not supported
,"fcvt_l_q"
,"fcvt_lu_q"
,"fcvt_q_l"
,"fcvt_q_lu"

};


std::vector<std::string> Itypes { // imm[11:0] rs1 funct3 rd opcode
// rv32i
"jalr"
,"lb"
,"lh"
,"lw"
,"lbu"
,"lhu"
,"addi"
,"slti"
,"sltiu"
,"xori"
,"ori"
,"andi"
,"ecall"
,"ebreak"

// RV32/RV64 Zifencei
,"fence_i"

// rv64i
,"lwu"
,"ld"
,"addiw"

// rv32f
,"flw"

// rv32d
,"fld"

};

// Actually I type
std::vector<std::string> Ifencetypes {
// rv32i
"fence" // fm pred succ rs1 funct3 rd opcode // fm pred succ rs1 000 rd 0001111
};

// Actually I type
std::vector<std::string> Icsrrs1types { // imm[11:0] rs1 funct3 rd opcode // csr rs1 funct3 rd opcode
// RV32/RV64 Zicsr
"csrrw"
,"csrrs"
,"csrrc"
};

// Actually I type
std::vector<std::string> Icsruimmtypes { // imm[11:0] rs1 funct3 rd opcode // csr uimm funct3 rd opcode
// RV32/RV64 Zicsr
"csrrwi"
,"csrrsi"
,"csrrci"
};

// Actually I type
std::vector<std::string> Ishtypes { // imm[11:0] rs1 funct3 rd opcode // funct6 shamt rs1 funct3 rd opcode
// rv64i
// not supported
"slli"
,"srli"
,"srai"

};

std::vector<std::string> Stypes { // imm[11:5] rs2 rs1 funct3 imm[4:0] opcode
// rv32i
"sb"
,"sh"
,"sw"

// rv64i
,"sd"

// rv64m
,"fsw"

// rv32d
,"fsd"

// rv32q
// not supported
,"fsq"

};

std::vector<std::string> Btypes { // imm[12|10:5] rs2 rs1 funct3 imm[4:1|11] opcode
// rv32i
"beq"
,"bne"
,"blt"
,"bge"
,"bltu"
,"bgeu"

};

std::vector<std::string> Utypes { // imm[31:12] rd opcode
// rv32i
"lui"
,"auipc"

};

std::vector<std::string> Jtypes { // imm[20|10:1|11|19:12] rd opcode
// rv32i
"jal"

};

std::vector<std::string> R4types { // rs3 funct2 rs2 rs1 funct3 rd opcode // rs3 funct2 rs2 rs1 rm rd opcode
// rv32f
"fmadd_s"
,"fmsub_s"
,"fnmsub_s"
,"fnmadd_s"

// rv32d
,"fmadd_d"
,"fmsub_d"
,"fnmsub_d"
,"fnmadd_d"

// rv32q
// not supported
,"fmadd_q"
,"fmsub_q"
,"fnmsub_q"
,"fnmadd_q"

};

std::vector<std::string> opcodes_alli{};


const int mostLeftBitBtn = 10;
const int mostTopBitBtn = 100;
const int bitBtnWidth = 26;
const int bitBtnHeight = 26;

const int bitBtnIDOffset = 1000;
const int blockBtnIDOffset = 2000;

int instBitLength = 32;

class instPartBlock{
    private:
        std::string blockStr;
        int bitButtonCount;
        
    public:
        std::string getBlockString() {return blockStr;}
        int getBitButtonCount() {return bitButtonCount;}

        void setBlockString(std::string blockStr) {
            this->blockStr = blockStr;
        }

        void setBitButtonCount(int bitButtonCount) {
            this->bitButtonCount = bitButtonCount;
        }
};

class instParts{
    private:
        std::string instName;
        std::string instType;
        int blockNum;
        std::vector<std::string> blockNames;
        std::vector<int> bitBtnCounts;

    public:
        std::string getInstName() {return instName;}
        std::string getInstType() {return instType;}
        int getBlockNum() {return blockNum;}
        std::vector<std::string> getBlockNames() {return blockNames;}

        std::string getBlockNamesnth(int nth) {return blockNames[nth];}
        int getBitBtnCountsnth(int nth) {return bitBtnCounts[nth];}

        void setInstName(std::string instName) {
            this->instName = instName;
        }
        void setInstType(std::string instType) {
            this->instType = instType;
        }
        void setBlockNum(int blockNum) {
            this->blockNum = blockNum;
        }
        /*
        void setBlockNames(std::vector<std::string>) {
            this->blockNames = blockNames;
        }*/

    instParts(std::string instName, std::string instType, std::vector<std::string> blockNames, std::vector<int> bitBtnCounts){
        this->instName = instName;
        this->instType = instType;
        this->blockNames = blockNames;
        this->blockNum = blockNames.size();
        this->bitBtnCounts = bitBtnCounts;
    }

};


// multiple definition
void vectorAddi(std::vector<std::string> &vector1, std::vector<std::string> &vector2)
{
    vector1.insert( vector1.end(), vector2.begin(), vector2.end() );
}

encIDESelBits::encIDESelBits(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    vectorAddi(opcodes_alli, Rtypes);
    vectorAddi(opcodes_alli, Rshtypes);
    vectorAddi(opcodes_alli, Raqtypes);
    vectorAddi(opcodes_alli, Rrmtypes);
    vectorAddi(opcodes_alli, Itypes);
    vectorAddi(opcodes_alli, Ifencetypes);
    vectorAddi(opcodes_alli, Icsrrs1types);
    vectorAddi(opcodes_alli, Icsruimmtypes);
    vectorAddi(opcodes_alli, Ishtypes);
    vectorAddi(opcodes_alli, Stypes);
    vectorAddi(opcodes_alli, Btypes);
    vectorAddi(opcodes_alli, Utypes);
    vectorAddi(opcodes_alli, Jtypes);
    vectorAddi(opcodes_alli, R4types);

    SetBackgroundColour(wxColor(180, 181, 185));

    this->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false));

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));

	//SetClientSize(wxSize(790,450));
    SetClientSize(wxSize(55*16,55*9));

    Choice1 = new wxComboBox(this, ID_CHOICE1, "", wxPoint(136,64), wxDefaultSize, 0, NULL, 0, wxDefaultValidator, _T("ID_CHOICE1"));

	//Choice1 = new wxChoice(this, ID_CHOICE1, wxPoint(136,64), wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Select Instruction:"), wxPoint(8,64), wxDefaultSize, 0, _T("ID_STATICTEXT1"));

	Connect(ID_CHOICE1,wxEVT_COMMAND_COMBOBOX_SELECTED,(wxObjectEventFunction)&encIDESelBits::OnChoice1Select);

    //Connect(ID_CHOICE1,wxEVT_TEXT,(wxObjectEventFunction)&encIDESelBits::OnChoiceTextUpdated);

	goBckBtn = new wxButton(this, ID_BUTTON1, _("Go back"), wxPoint(8,8), wxSize(88,29), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&encIDESelBits::OngoBckBtnClick);

    lbl1 = new wxStaticText(this, ID_STATICTEXT2, std::string(instBitLength, '0'), wxPoint(192,8), wxSize(768,32), 0, _T("ID_STATICTEXT2"));

    addAlp = new wxButton(this, ID_BUTTON3, _("Add flag"), wxPoint(688,8), wxSize(88,29), 0, wxDefaultValidator, _T("ID_BUTTON3"));
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&encIDESelBits::OnaddAlpBtnClick);

    for(wxInt8 i=0; i<opcodes_alli.size(); i++){
        Choice1->Append(opcodes_alli[i]);
    }

    int left = mostLeftBitBtn;
    int top = mostTopBitBtn;
    int width = bitBtnWidth;
    int height = bitBtnHeight;

    wxButton* bitBtns[instBitLength];
    for(wxInt8 i=0; i<instBitLength; i++){
        if(left > this->GetSize().GetWidth() - width){
            left = mostLeftBitBtn;
            top += 1.7 * bitBtnHeight; // 24*1.7=40 // decrease top btn position when a row has ended
        }

        bitBtns[i] = new wxButton(this, bitBtnIDOffset + i, wxString::Format(wxT("%i"), 0), wxPoint(left, top), wxSize(width, height), 0, wxDefaultValidator, wxString::Format(wxT("%i"), 0));
        bitBtns[i]->Bind(wxEVT_BUTTON, &encIDESelBits::OnBitButtons, this);
        bitBtns[i]->SetBackgroundColour(*wxRED);
        left += width;
    }

    //Choice1->SetSelection(0);
    //CreateDynamicButtons(this, "lui");
    //CreateDynamicButtons(this, "add");

    instTypeLbl = new wxStaticText(this, 1500, _(""), wxPoint(8,190), wxDefaultSize, 0, _T("ID_STATICTEXT"));

    instTypeLbl->SetLabel("");

    // , wxT("Tahoma")

    //wxFont font(5, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
    //wxFont font(16, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Tahoma"));

    //wxGetActiveWindow()->SetFont(wxGetActiveWindow()->GetFont().Scale(0.5));

    //this->SetFont(this->GetFont().Scale(3));
    //this->SetFont(font);
}

encIDESelBits::~encIDESelBits()
{

}

void encIDESelBits::OngoBckBtnClick(wxCommandEvent& event)
{
    encIDEEnc* encFrame = new encIDEEnc(0);
    encFrame->CenterOnParent();
    encFrame->Show();
    this->Close();
}

void encIDESelBits::OnBitButtons(wxCommandEvent & event)
{
    wxButton *bitBtn = wxDynamicCast(event.GetEventObject(), wxButton);

    wxString bitLbl = lbl1->GetLabel();

    auto index = bitBtn->GetId() - bitBtnIDOffset;

    if(bitLbl[index].GetValue() == '0'){
        bitLbl.replace(index,1,"1");
        bitBtn->SetBackgroundColour(*wxGREEN);
        bitBtn->SetLabel("1");
    }
    else{
        bitLbl.replace(index,1,"0");
        bitBtn->SetBackgroundColour(*wxRED);
        bitBtn->SetLabel("0");
    }
    lbl1->SetLabel(bitLbl);
}

std::string inst = "";

void encIDESelBits::OnChoice1Select(wxCommandEvent& event)
{
    inst = opcodes_alli[Choice1->GetSelection()];
    RemoveAllDynamicButtons(this, inst);
    //CreateDynamicButtons(this, inst);

    instTypeLbl->SetLabel(CreateDynamicButtons(inst));
}

bool isStringInVector(std::vector<std::string> vect, std::string str){
    auto it = std::find(vect.begin(), vect.end(), str);
    if (it != vect.end())
    {
      return true;
    } else
    {
      return false;
    }
}

void encIDESelBits::OnChoiceTextUpdated(wxCommandEvent& event){
    if(isStringInVector(opcodes_alli, Choice1->GetValue().ToStdString()))
        Choice1->SetStringSelection(Choice1->GetValue().ToStdString());
}

void encIDESelBits::OnaddAlpBtnClick(wxCommandEvent& event)
{
    if(inst != ""){
        wxGetApp().compileOptionsString += " --b_p_" + inst + "=" + lbl1->GetLabelText() + " ";
        wxMessageBox(wxGetApp().compileOptionsString + " flags added.");
    }
}

enum instType {
    Rtype,
    Rshtype, Raqtype, Rrmtype,
    Itype,
    Ifencetype, Icsrrs1type, Icsruimmtype, Ishtype,
    Stype,
    Btype,
    Utype,
    Jtype,
    R4type
};

std::string encIDESelBits::CreateDynamicButtons(std::string inst){
    int left = mostLeftBitBtn;
    int top = mostTopBitBtn + 1.7*bitBtnHeight;
    int width = bitBtnWidth;
    int height = bitBtnHeight;

    int btnCount = 0;

    std::string instType = "";
    std::vector<std::string> blockNames;
    std::vector<int> bitBtnCounts;

    if(getInstType(inst) == Rtype){
        instType = "R-type";
        btnCount = 6;
        blockNames = {"funct7","rs2","rs1","funct3","rd","opcode"};
        bitBtnCounts = {7,5,5,3,5,7};
    }
    else if(getInstType(inst) == Rshtype){ // funct7 shamt rs1 funct3 rd opcode
        instType = "R-type";
        btnCount = 6;
        blockNames = {"funct7","shamt","rs1","funct3","rd","opcode"};
        bitBtnCounts = {7,5,5,3,5,7};
    }
    else if(getInstType(inst) == Raqtype){ // funct5 aq rl rs2 rs1 funct3 rd opcode
        instType = "R-type";
        btnCount = 8;
        blockNames = {"funct5","aq","rl","rs2","rs1","funct3","rd","opcode"};
        bitBtnCounts = {5,1,1,5,5,3,5,7};
    }
    else if(getInstType(inst) == Rrmtype){ // funct7 rs2 rs1 rm rd opcode
        instType = "R-type";
        btnCount = 6;
        blockNames = {"funct7","rs2","rs1","rm","rd","opcode"};
        bitBtnCounts = {7,5,5,3,5,7};
    }
    else if(getInstType(inst) == Itype){
        instType = "I-type";
        btnCount = 5;
        blockNames = {"imm[11:0]","rs1","funct3","rd","opcode"};
        bitBtnCounts = {12,5,3,5,7};
    }
    else if(getInstType(inst) == Ifencetype){ // fm pred succ rs1 funct3 rd opcode
        instType = "I-type";
        btnCount = 7;
        blockNames = {"fm","pred","succ","rs1","funct3","rd","opcode"};
        bitBtnCounts = {4,4,4,5,3,5,7};
    }
    else if(getInstType(inst) == Icsrrs1type){ // csr rs1 funct3 rd opcode
        instType = "I-type";
        btnCount = 5;
        blockNames = {"csr","rs1","funct3","rd","opcode"};
        bitBtnCounts = {12,5,3,5,7};
    }
    else if(getInstType(inst) == Icsruimmtype){ // csr uimm funct3 rd opcode
        instType = "I-type";
        btnCount = 5;
        blockNames = {"csr","uimm","funct3","rd","opcode"};
        bitBtnCounts = {12,5,3,5,7};
    }
    else if(getInstType(inst) == Ishtype){ // funct6 shamt rs1 funct3 rd opcode
        instType = "I-type";
        btnCount = 6;
        blockNames = {"funct6","shamt","rs1","funct3","rd","opcode"};
        bitBtnCounts = {6,6,5,3,5,7};
    }

    else if(getInstType(inst) == Stype){
        instType = "S-type";
        btnCount = 6;
        blockNames = {"imm[11:5]","rs2","rs1","funct3","imm[4:0]","opcode"};
        bitBtnCounts = {7,5,5,3,5,7};
    }
    else if(getInstType(inst) == Btype){
        instType = "B-type";
        btnCount = 6;
        blockNames = {"imm[12|10:5]","rs2","rs1","funct3","imm[4:1|11]","opcode"};
        bitBtnCounts = {7,5,5,3,5,7};
    }
    else if(getInstType(inst) == Utype){
        instType = "U-type";
        btnCount = 3;
        blockNames = {"imm[31:12]","rd","opcode"};
        bitBtnCounts = {20,5,7};
    }
    else if(getInstType(inst) == Jtype){
        instType = "J-type";
        btnCount = 3;
        blockNames = {"imm[20|10:1|11|19:12]","rd","opcode"};
        bitBtnCounts = {20,5,7};
    }
    else if(getInstType(inst) == R4type){
        instType = "R4-type";
        btnCount = 7;
        blockNames = {"rs3","funct2","rs2","rs1","funct3","rd","opcode"};
        bitBtnCounts = {5,2,5,5,3,5,7};
    }

    else if(inst == "c_addi4spn"){ // 000 nzuimm[5:4|9:6|2|3] rd ' 00
        instType = "rvcq0";
        btnCount = 4;
        blockNames = {"funct3", "nzuimm[5:4|9:6|2|3]", "rd'", "opcode"};
        bitBtnCounts = {3, 8, 3, 2};
    }
    else if(inst == "c_fld"){ // 001 uimm[5:3] rs1 ' uimm[7:6] rd ' 00
        instType = "rvcq0";
        btnCount = 6;
        blockNames = {"funct3", "uimm[5:3]", "rs1'", "uimm[7:6]", "rd'", "opcode"};
        bitBtnCounts = {3, 3, 3, 2, 3, 2};
    }
    else if(inst == "c_lq"){ // 001 uimm[5:4|8] rs1 ' uimm[7:6] rd ' 00
        instType = "rvcq0";
        btnCount = 6;
        blockNames = {"funct3", "uimm[5:4|8]", "rs1'", "uimm[7:6]", "rd'", "opcode"};
        bitBtnCounts = {3, 3, 3, 2, 3, 2};
    }
    else if(inst == "c_lw"){ // 010 uimm[5:3] rs1 ' uimm[2|6] rd ' 00
        instType = "rvcq0";
        btnCount = 6;
        blockNames = {"funct3", "uimm[5:3]", "rs1'", "uimm[2|6]", "rd'", "opcode"};
        bitBtnCounts = {3, 3, 3, 2, 3, 2};
    }
    else if(inst == "c_flw"){ // 011 uimm[5:3] rs1 ' uimm[2|6] rd ' 00
        instType = "rvcq0";
        btnCount = 6;
        blockNames = {"funct3", "uimm[5:3]", "rs1'", "uimm[2|6]", "rd'", "opcode"};
        bitBtnCounts = {3, 3, 3, 2, 3, 2};
    }
    else if(inst == "c_ld"){ // 011 uimm[5:3] rs1 ' uimm[7:6] rd ' 00
        instType = "rvcq0";
        btnCount = 6;
        blockNames = {"funct3", "uimm[5:3]", "rs1'", "uimm[7:6]", "rd'", "opcode"};
        bitBtnCounts = {3, 3, 3, 2, 3, 2};
    }
    else if(inst == "c_fsd"){ // 101 uimm[5:3] rs1 ' uimm[7:6] rs2 ' 00
        instType = "rvcq0";
        btnCount = 6;
        blockNames = {"funct3", "uimm[5:3]", "rs1'", "uimm[7:6]", "rs2'", "opcode"};
        bitBtnCounts = {3, 3, 3, 2, 3, 2};
    }
    else if(inst == "c_sq"){ // 101 uimm[5:4|8] rs1 ' uimm[7:6] rs2 ' 00
        instType = "rvcq0";
        btnCount = 6;
        blockNames = {"funct3", "uimm[5:4|8]", "rs1'", "uimm[7:6]", "rs2'", "opcode"};
        bitBtnCounts = {3, 3, 3, 2, 3, 2};
    }
    else if(inst == "c_sw"){ // 110 uimm[5:3] rs1 ' uimm[2|6] rs2 ' 00
        instType = "rvcq0";
        btnCount = 6;
        blockNames = {"funct3", "uimm[5:3]", "rs1'", "uimm[2|6]", "rs2'", "opcode"};
        bitBtnCounts = {3, 3, 3, 2, 3, 2};
    }
    else if(inst == "c_fsw"){ // 111 uimm[5:3] rs1 ' uimm[2|6] rs2 ' 00
        instType = "rvcq0";
        btnCount = 6;
        blockNames = {"funct3", "uimm[5:3]", "rs1'", "uimm[2|6]", "rs2'", "opcode"};
        bitBtnCounts = {3, 3, 3, 2, 3, 2};
    }
    else if(inst == "c_sd"){ // 111 uimm[5:3] rs1 ' uimm[7:6] rs2 ' 00
        instType = "rvcq0";
        btnCount = 6;
        blockNames = {"funct3", "uimm[5:3]", "rs1'", "uimm[7:6]", "rs2'", "opcode"};
        bitBtnCounts = {3, 3, 3, 2, 3, 2};
    }

    else if(inst == "c_nop"){ // 000 nzimm[5] 0 nzimm[4:0] 01
        instType = "rvcq1";
        btnCount = 5;
        blockNames = {"funct3", "nzimm[5]", "0", "nzimm[4:0]", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_addi"){ // 000 nzimm[5] rs1|rd|=0 nzimm[4:0] 01
        instType = "rvcq1";
        btnCount = 5;
        blockNames = {"funct3", "nzimm[5]", "rs1|rd|=0", "nzimm[4:0]", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_jal"){ // 001 imm[11|4|9:8|10|6|7|3:1|5] 01
        instType = "rvcq1";
        btnCount = 3;
        blockNames = {"funct3", "imm[11|4|9:8|10|6|7|3:1|5]", "opcode"};
        bitBtnCounts = {3, 11, 2};
    }
    else if(inst == "c_addiw"){ // 001 imm[5] rs1|rd|=0 imm[4:0] 01
        instType = "rvcq1";
        btnCount = 5;
        blockNames = {"funct3", "imm[5]", "rs1|rd|=0", "imm[4:0]", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_li"){ // 010 imm[5] rd|=0 imm[4:0] 01
        instType = "rvcq1";
        btnCount = 5;
        blockNames = {"funct3", "imm[5]", "rd|=0", "imm[4:0]", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_addi16sp"){ // 011 nzimm[9] 2 nzimm[4|6|8:7|5] 01
        instType = "rvcq1";
        btnCount = 5;
        blockNames = {"funct3", "nzimm[9]", "2", "nzimm[4|6|8:7|5]", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_lui"){ // 011 nzimm[17] rd|={0, 2} nzimm[16:12] 01
        instType = "rvcq1";
        btnCount = 5;
        blockNames = {"funct3", "nzimm[17]", "rd|={0, 2}", "nzimm[16:12]", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_srli"){ // 100 nzuimm[5] 00 rs1 '/rd ' nzuimm[4:0] 01
        instType = "rvcq1";
        btnCount = 6;
        blockNames = {"funct3", "nzuimm[5]", "funct2", "rs1'|rd'", "nzuimm[4:0]", "opcode"};
        bitBtnCounts = {3, 1, 2, 3, 5, 2};
    }
    else if(inst == "c_srli64"){ // 100 0 00 rs1 '/rd ' 0 01
        instType = "rvcq1";
        btnCount = 6;
        blockNames = {"funct3", "0", "funct2", "rs1'|rd'", "0", "opcode"};
        bitBtnCounts = {3, 1, 2, 3, 5, 2};
    }
    else if(inst == "c_srai"){ // 100 nzuimm[5] 01 rs1 '/rd ' nzuimm[4:0] 01
        instType = "rvcq1";
        btnCount = 6;
        blockNames = {"funct3", "nzuimm[5]", "funct2", "rs1'|rd'", "nzuimm[4:0]", "opcode"};
        bitBtnCounts = {3, 1, 2, 3, 5, 2};
    }
    else if(inst == "c_srai64"){ // 100 0 01 rs1 '/rd ' 0 01
        instType = "rvcq1";
        btnCount = 6;
        blockNames = {"funct3", "0", "funct2", "rs1'|rd'", "0", "opcode"};
        bitBtnCounts = {3, 1, 2, 3, 5, 2};
    }
    else if(inst == "c_andi"){ // 100 imm[5] 10 rs1 '/rd ' imm[4:0] 01
        instType = "rvcq1";
        btnCount = 6;
        blockNames = {"funct3", "imm[5]", "funct2", "rs1'|rd'", "imm[4:0]", "opcode"};
        bitBtnCounts = {3, 1, 2, 3, 5, 2};
    }
    else if(inst == "c_sub"){ // 100 0 11 rs1 '/rd ' 00 rs2 ' 01
        instType = "rvcq1";
        btnCount = 7;
        blockNames = {"funct3", "0", "funct2", "rs1'|rd'", "funct2", "rs2'", "opcode"};
        bitBtnCounts = {3, 1, 2, 3, 2, 3, 2};
    }
    else if(inst == "c_xor"){ // 100 0 11 rs1 '/rd ' 01 rs2 ' 01
        instType = "rvcq1";
        btnCount = 7;
        blockNames = {"funct3", "0", "funct2", "rs1'|rd'", "funct2", "rs2'", "opcode"};
        bitBtnCounts = {3, 1, 2, 3, 2, 3, 2};
    }
    else if(inst == "c_or"){ // 100 0 11 rs1 '/rd ' 10 rs2 ' 01
        instType = "rvcq1";
        btnCount = 7;
        blockNames = {"funct3", "0", "funct2", "rs1'|rd'", "funct2", "rs2'", "opcode"};
        bitBtnCounts = {3, 1, 2, 3, 2, 3, 2};
    }
    else if(inst == "c_and"){ // 100 0 11 rs1 '/rd ' 11 rs2 ' 01
        instType = "rvcq1";
        btnCount = 7;
        blockNames = {"funct3", "0", "funct2", "rs1'|rd'", "funct2", "rs2'", "opcode"};
        bitBtnCounts = {3, 1, 2, 3, 2, 3, 2};
    }
    else if(inst == "c_subw"){ // 100 1 11 rs1 '/rd ' 00 rs2 ' 01
        instType = "rvcq1";
        btnCount = 7;
        blockNames = {"funct3", "1", "funct2", "rs1'|rd'", "funct2", "rs2'", "opcode"};
        bitBtnCounts = {3, 1, 2, 3, 2, 3, 2};
    }
    else if(inst == "c_addw"){ // 100 1 11 rs1 '/rd ' 01 rs2 ' 01
        instType = "rvcq1";
        btnCount = 7;
        blockNames = {"funct3", "1", "funct2", "rs1'|rd'", "funct2", "rs2'", "opcode"};
        bitBtnCounts = {3, 1, 2, 3, 2, 3, 2};
    }
    else if(inst == "c_j"){ // 101 imm[11|4|9:8|10|6|7|3:1|5] 01
        instType = "rvcq1";
        btnCount = 3;
        blockNames = {"funct3", "imm[11|4|9:8|10|6|7|3:1|5]", "opcode"};
        bitBtnCounts = {3, 11, 2};
    }
    else if(inst == "c_beqz"){ // 110 imm[8|4:3] rs1 ' imm[7:6|2:1|5] 01
        instType = "rvcq1";
        btnCount = 5;
        blockNames = {"funct3", "imm[8|4:3]", "rs1'", "imm[7:6|2:1|5]", "opcode"};
        bitBtnCounts = {3, 3, 3, 5, 2};
    }
    else if(inst == "c_bnez"){ // 111 imm[8|4:3] rs1 ' imm[7:6|2:1|5] 01
        instType = "rvcq1";
        btnCount = 5;
        blockNames = {"funct3", "imm[8|4:3]", "rs1'", "imm[7:6|2:1|5]", "opcode"};
        bitBtnCounts = {3, 3, 3, 5, 2};
    }

    else if(inst == "c_slli"){ // 000 nzuimm[5] rs1|rd|=0 nzuimm[4:0] 10
        instType = "rvcq2";
        btnCount = 5;
        blockNames = {"funct3", "nzuimm[5]", "rs1|rd|=0", "nzuimm[4:0]", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_slli64"){ // 000 0 rs1|rd|=0 0 10
        instType = "rvcq2";
        btnCount = 5;
        blockNames = {"funct3", "0", "rs1|rd|=0", "0", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_fldsp"){ // 001 uimm[5] rd uimm[4:3|8:6] 10
        instType = "rvcq2";
        btnCount = 5;
        blockNames = {"funct3", "uimm[5]", "rd", "uimm[4:3|8:6]", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_lqsp"){ // 001 uimm[5] rd|=0 uimm[4|9:6] 10
        instType = "rvcq2";
        btnCount = 5;
        blockNames = {"funct3", "uimm[5]", "rd|=0", "uimm[4|9:6]", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_lwsp"){ // 010 uimm[5] rd|=0 uimm[4:2|7:6] 10
        instType = "rvcq2";
        btnCount = 5;
        blockNames = {"funct3", "uimm[5]", "rd|=0", "uimm[4:2|7:6]", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_flwsp"){ // 011 uimm[5] rd uimm[4:2|7:6] 10
        instType = "rvcq2";
        btnCount = 5;
        blockNames = {"funct3", "uimm[5]", "rd", "uimm[4:2|7:6]", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_ldsp"){ // 011 uimm[5] rd|=0 uimm[4:3|8:6] 10
        instType = "rvcq2";
        btnCount = 5;
        blockNames = {"funct3", "uimm[5]", "rd|=0", "uimm[4:3|8:6]", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_jr"){ // 100 0 rs1|=0 0 10
        instType = "rvcq2";
        btnCount = 5;
        blockNames = {"funct3", "0", "rs1|=0", "0", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_mv"){ // 100 0 rd|=0 rs2|=0 10
        instType = "rvcq2";
        btnCount = 5;
        blockNames = {"funct3", "0", "rd|=0", "rs2|=0", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_ebreak"){ // 100 1 0 0 10
        instType = "rvcq2";
        btnCount = 5;
        blockNames = {"funct3", "1", "0", "0", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_jalr"){ // 100 1 rs1|=0 0 10
        instType = "rvcq2";
        btnCount = 5;
        blockNames = {"funct3", "1", "rs1|=0", "0", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_add"){ // 100 1 rs1|rd|=0 rs2|=0 10
        instType = "rvcq2";
        btnCount = 5;
        blockNames = {"funct3", "1", "rs1|rd|=0", "rs2|=0", "opcode"};
        bitBtnCounts = {3, 1, 5, 5, 2};
    }
    else if(inst == "c_fsdsp"){ // 101 uimm[5:3|8:6] rs2 10
        instType = "rvcq2";
        btnCount = 4;
        blockNames = {"funct3", "uimm[5:3|8:6]", "rs2", "opcode"};
        bitBtnCounts = {3, 6, 5, 2};
    }
    else if(inst == "c_sqsp"){ // 101 uimm[5:4|9:6] rs2 10
        instType = "rvcq2";
        btnCount = 4;
        blockNames = {"funct3", "uimm[5:4|9:6]", "rs2", "opcode"};
        bitBtnCounts = {3, 6, 5, 2};
    }
    else if(inst == "c_swsp"){ // 110 uimm[5:2|7:6] rs2 10
        instType = "rvcq2";
        btnCount = 4;
        blockNames = {"funct3", "uimm[5:2|7:6]", "rs2", "opcode"};
        bitBtnCounts = {3, 6, 5, 2};
    }
    else if(inst == "c_fswsp"){ // 111 uimm[5:2|7:6] rs2 10
        instType = "rvcq2";
        btnCount = 4;
        blockNames = {"funct3", "uimm[5:2|7:6]", "rs2", "opcode"};
        bitBtnCounts = {3, 6, 5, 2};
    }
    else if(inst == "c_sdsp"){ // 111 uimm[5:3|8:6] rs2 10
        instType = "rvcq2";
        btnCount = 4;
        blockNames = {"funct3", "uimm[5:3|8:6]", "rs2", "opcode"};
        bitBtnCounts = {3, 6, 5, 2};
    }

    instParts partedInst(inst, instType, blockNames, bitBtnCounts);

    wxButton* btns[btnCount];

    int cumulative = 0;

    for(wxInt8 i=0; i<btnCount; i++){
        btns[i] = new wxButton(this, blockBtnIDOffset + i, partedInst.getBlockNamesnth(i), wxPoint(left + width * cumulative, top), wxSize(width * partedInst.getBitBtnCountsnth(i), height), 0, wxDefaultValidator, partedInst.getBlockNamesnth(i));
        cumulative += partedInst.getBitBtnCountsnth(i);
        btns[i]->Bind(wxEVT_BUTTON, &encIDESelBits::OnBlockButtons, this);
        btns[i]->SetBackgroundColour(*wxRED);
    }

    return instType;
}


void encIDESelBits::OnBlockButtons(wxCommandEvent & event)
{
    wxButton *blockBtn = wxDynamicCast(event.GetEventObject(), wxButton);

    wxString bitLbl = lbl1->GetLabel();

    int blockBtnLeftPos = blockBtn->GetPosition().x;
    int nthBitBtn = (blockBtnLeftPos - mostLeftBitBtn) / bitBtnWidth;
    int numOfBitBtnDepOnBlockBtnWidth = blockBtn->GetSize().GetWidth() / bitBtnWidth;

    for(wxInt8 index=nthBitBtn; index<(nthBitBtn+numOfBitBtnDepOnBlockBtnWidth); index++){
        wxButton* eachBitBtn = wxDynamicCast(wxWindow::FindWindowById(bitBtnIDOffset + index, this), wxButton);

        if(blockBtn->GetBackgroundColour() == *wxRED){
            bitLbl.replace(index,1,"1");
            eachBitBtn->SetLabel("1");
            eachBitBtn->SetBackgroundColour(*wxGREEN);
        }
        else{
            bitLbl.replace(index,1,"0");
            eachBitBtn->SetLabel("0");
            eachBitBtn->SetBackgroundColour(*wxRED);
        }
    }

    lbl1->SetLabel(bitLbl);

    if(blockBtn->GetBackgroundColour() == *wxRED){
        blockBtn->SetBackgroundColour(*wxGREEN);
    }
    else{
        blockBtn->SetBackgroundColour(*wxRED);
    }
}

void RemoveAllDynamicButtons(encIDESelBits *thisWindow, std::string inst){
    int btnCount = 0;

    if(getInstType(inst) == Rtype){
        btnCount = 6;
    }
    else if(getInstType(inst) == Raqtype){
        btnCount = 8;
    }
    else if(getInstType(inst) == Itype){
        btnCount = 5;
    }
    else if(getInstType(inst) == Stype){
        btnCount = 6;
    }
    else if(getInstType(inst) == Btype){
        btnCount = 6;
    }
    else if(getInstType(inst) == Utype){
        btnCount = 3;
    }
    else if(getInstType(inst) == Jtype){
        btnCount = 3;
    }
    else if(getInstType(inst) == R4type){
        btnCount = 5;
    }

    for(wxInt8 i=0; i<btnCount; i++){
        if(wxWindow* ctrl = wxWindow::FindWindowById(blockBtnIDOffset + i, thisWindow)){
            ctrl->Destroy();
        }
    }
}




// "rs1'"

int getInstType(std::string inst){
    if (std::find(Rtypes.begin(), Rtypes.end(), inst) != Rtypes.end())
        return Rtype;
    else if (std::find(Rshtypes.begin(), Rshtypes.end(), inst) != Rshtypes.end())
        return Rshtype;
    else if (std::find(Raqtypes.begin(), Raqtypes.end(), inst) != Raqtypes.end())
        return Raqtype;
    else if (std::find(Rrmtypes.begin(), Rrmtypes.end(), inst) != Rrmtypes.end())
        return Rrmtype;
    else if (std::find(Itypes.begin(), Itypes.end(), inst) != Itypes.end())
        return Itype;
    else if (std::find(Ifencetypes.begin(), Ifencetypes.end(), inst) != Ifencetypes.end())
        return Ifencetype;
    else if (std::find(Icsrrs1types.begin(), Icsrrs1types.end(), inst) != Icsrrs1types.end())
        return Icsrrs1type;
    else if (std::find(Icsruimmtypes.begin(), Icsruimmtypes.end(), inst) != Icsruimmtypes.end())
        return Icsruimmtype;
    else if (std::find(Ishtypes.begin(), Ishtypes.end(), inst) != Ishtypes.end())
        return Ishtype;
    else if (std::find(Stypes.begin(), Stypes.end(), inst) != Stypes.end())
        return Stype;
    else if (std::find(Btypes.begin(), Btypes.end(), inst) != Btypes.end())
        return Btype;
    else if (std::find(Utypes.begin(), Utypes.end(), inst) != Utypes.end())
        return Utype;
    else if (std::find(Jtypes.begin(), Jtypes.end(), inst) != Jtypes.end())
        return Jtype;
    else if (std::find(R4types.begin(), R4types.end(), inst) != R4types.end())
        return R4type;
    else return Rtype;
}
