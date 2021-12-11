#include "encIDEenc.h"
#include "encIDEMain.h"

#include "encIDEApp.h"
//#include "encIDEMain.cpp"

#include <wx/msgdlg.h>


//(*InternalHeaders(encIDEenc)
#include <wx/string.h>
#include <wx/intl.h>
//*)

//(*IdInit(encIDEenc)

//*)

const long encIDEenc::ID_CHK_ienc32insts = wxNewId();
const long encIDEenc::ID_CHK_menc32insts = wxNewId();
const long encIDEenc::ID_CHK_aenc32insts = wxNewId();
const long encIDEenc::ID_CHK_fenc32insts = wxNewId();
const long encIDEenc::ID_CHK_denc32insts = wxNewId();
const long encIDEenc::ID_CHK_qenc32insts = wxNewId();

const long encIDEenc::ID_CHK_ienc64insts = wxNewId();
const long encIDEenc::ID_CHK_menc64insts = wxNewId();
const long encIDEenc::ID_CHK_aenc64insts = wxNewId();
const long encIDEenc::ID_CHK_fenc64insts = wxNewId();
const long encIDEenc::ID_CHK_denc64insts = wxNewId();
const long encIDEenc::ID_CHK_qenc64insts = wxNewId();

const long encIDEenc::ID_CHK_cencq2insts = wxNewId();
const long encIDEenc::ID_CHK_cencq1insts = wxNewId();
const long encIDEenc::ID_CHK_cencq0insts = wxNewId();

const long encIDEenc::ID_BUTTON1 = wxNewId();
const long encIDEenc::ID_BUTTON2 = wxNewId();
const long encIDEenc::ID_BUTTON3 = wxNewId();
//const long encIDEenc::ID_STATICTEXT1 = wxNewId();
const long encIDEenc::ID_LBL_ienc32insts = wxNewId();
const long encIDEenc::ID_LBL_menc32insts = wxNewId();
const long encIDEenc::ID_LBL_aenc32insts = wxNewId();
const long encIDEenc::ID_LBL_fenc32insts = wxNewId();
const long encIDEenc::ID_LBL_denc32insts = wxNewId();
const long encIDEenc::ID_LBL_qenc32insts = wxNewId();

const long encIDEenc::ID_LBL_ienc64insts = wxNewId();
const long encIDEenc::ID_LBL_menc64insts = wxNewId();
const long encIDEenc::ID_LBL_aenc64insts = wxNewId();
const long encIDEenc::ID_LBL_fenc64insts = wxNewId();
const long encIDEenc::ID_LBL_denc64insts = wxNewId();
const long encIDEenc::ID_LBL_qenc64insts = wxNewId();

const long encIDEenc::ID_LBL_cencq2insts = wxNewId();
const long encIDEenc::ID_LBL_cencq1insts = wxNewId();
const long encIDEenc::ID_LBL_cencq0insts = wxNewId();

const long encIDEenc::ID_key_text = wxNewId();
const long encIDEenc::ID_keyall_text = wxNewId();


BEGIN_EVENT_TABLE(encIDEenc,wxFrame)
	//(*EventTable(encIDEenc)
	//*)
	//EVT_BUTTON(encIDEenc::OnButtons)
END_EVENT_TABLE()


void CreateDynamicButtons(encIDEenc *thisWindow, std::vector<std::string> opcodes);

std::vector<std::string> opcodes_rv32i { // + rv64i
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
,"csrrci"};

std::vector<std::string> opcodes_all{};

encIDEenc::encIDEenc(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(encIDEenc)
	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
	SetClientSize(wxSize(834,450));

	//*)

	opcodes_all = opcodes_rv32i;

	CHK_ienc32insts = new wxCheckBox(this, ID_CHK_ienc32insts, _("rv32i"), wxPoint(190, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_ienc32insts"));
	CHK_menc32insts = new wxCheckBox(this, ID_CHK_menc32insts, _("rv32m"), wxPoint(190+65, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_menc32insts"));
	CHK_aenc32insts = new wxCheckBox(this, ID_CHK_aenc32insts, _("rv32a"), wxPoint(190+65*2, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_aenc32insts"));
	CHK_fenc32insts = new wxCheckBox(this, ID_CHK_fenc32insts, _("rv32f"), wxPoint(190+65*3, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_fenc32insts"));
	CHK_denc32insts = new wxCheckBox(this, ID_CHK_denc32insts, _("rv32d"), wxPoint(190+65*4, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_denc32insts"));
	//CHK_qenc32insts = new wxCheckBox(this, ID_CHK_qenc32insts, _("rv32q"), wxPoint(190+65, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_qenc32insts"));

	CHK_ienc64insts = new wxCheckBox(this, ID_CHK_ienc64insts, _("rv64i"), wxPoint(190+65*5, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_ienc64insts"));
	CHK_menc64insts = new wxCheckBox(this, ID_CHK_menc64insts, _("rv64m"), wxPoint(190+65*6, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_menc64insts"));
	CHK_aenc64insts = new wxCheckBox(this, ID_CHK_aenc64insts, _("rv64a"), wxPoint(190+65*7, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_aenc64insts"));
	CHK_fenc64insts = new wxCheckBox(this, ID_CHK_fenc64insts, _("rv64f"), wxPoint(190+65*8, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_fenc64insts"));
	CHK_denc64insts = new wxCheckBox(this, ID_CHK_denc64insts, _("rv64d"), wxPoint(190+65*9, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_denc64insts"));
	//CHK_qenc64insts = new wxCheckBox(this, ID_CHK_qenc64insts, _("rv64q"), wxPoint(190+65, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_qenc64insts"));

	CHK_cencq0insts = new wxCheckBox(this, ID_CHK_cencq0insts, _("rvcq0"), wxPoint(190+65*10, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_cencq0insts"));
	CHK_cencq1insts = new wxCheckBox(this, ID_CHK_cencq1insts, _("rvcq1"), wxPoint(190+65*11, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_cencq1insts"));
	CHK_cencq2insts = new wxCheckBox(this, ID_CHK_cencq2insts, _("rvcq2"), wxPoint(190+65*12, 4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHK_cencq2insts"));

	CHK_ienc32insts->SetValue(true);
	CHK_menc32insts->SetValue(false);
	CHK_aenc32insts->SetValue(false);
	CHK_fenc32insts->SetValue(false);
	CHK_denc32insts->SetValue(false);
	//CHK_qenc32insts->SetValue(false);

	CHK_ienc64insts->SetValue(false);
	CHK_menc64insts->SetValue(false);
	CHK_aenc64insts->SetValue(false);
	CHK_fenc64insts->SetValue(false);
	CHK_denc64insts->SetValue(false);
	//CHK_qenc64insts->SetValue(false);

	CHK_cencq0insts->SetValue(false);
	CHK_cencq1insts->SetValue(false);
	CHK_cencq2insts->SetValue(false);


    SetClientSize(wxSize(1400,900));

	goBckBtn = new wxButton(this, ID_BUTTON1, _("Go back"), wxPoint(8,8), wxSize(88,29), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button1 = new wxButton(this, ID_BUTTON2, _("Select bits"), wxPoint(104,8), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    addAlp = new wxButton(this, ID_BUTTON3, _("Add flag"), wxPoint(1300,2), wxSize(88,29), 0, wxDefaultValidator, _T("ID_BUTTON3"));

    LBL_ienc32insts = new wxStaticText(this, ID_LBL_ienc32insts, std::string(47, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_ienc32insts"));
	LBL_menc32insts = new wxStaticText(this, ID_LBL_menc32insts, std::string(8 , '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_menc32insts"));
	LBL_aenc32insts = new wxStaticText(this, ID_LBL_aenc32insts, std::string(11, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_aenc32insts"));
	LBL_fenc32insts = new wxStaticText(this, ID_LBL_fenc32insts, std::string(26, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_fenc32insts"));
	LBL_denc32insts = new wxStaticText(this, ID_LBL_denc32insts, std::string(26, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_denc32insts"));
	//LBL_qenc32insts = new wxStaticText(this, ID_LBL_qenc32insts, std::string(28, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_qenc32insts"));

	LBL_ienc64insts = new wxStaticText(this, ID_LBL_ienc64insts, std::string(12, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_ienc64insts"));
	LBL_menc64insts = new wxStaticText(this, ID_LBL_menc64insts, std::string(5 , '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_menc64insts"));
	LBL_aenc64insts = new wxStaticText(this, ID_LBL_aenc64insts, std::string(11, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_aenc64insts"));
	LBL_fenc64insts = new wxStaticText(this, ID_LBL_fenc64insts, std::string(4 , '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_fenc64insts"));
	LBL_denc64insts = new wxStaticText(this, ID_LBL_denc64insts, std::string(6 , '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_denc64insts"));
	//LBL_qenc64insts = new wxStaticText(this, ID_LBL_qenc64insts, std::string(4 , '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_qenc64insts"));

	LBL_cencq0insts = new wxStaticText(this, ID_LBL_cencq0insts, std::string(11, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_cencq0insts"));
	LBL_cencq1insts = new wxStaticText(this, ID_LBL_cencq1insts, std::string(21, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_cencq1insts"));
	LBL_cencq2insts = new wxStaticText(this, ID_LBL_cencq2insts, std::string(17, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_cencq2insts"));

	LBL_ienc32insts->SetLabel(std::string(47, '0'));
	LBL_menc32insts->SetLabel("");
	LBL_aenc32insts->SetLabel("");
	LBL_fenc32insts->SetLabel("");
	LBL_denc32insts->SetLabel("");
	//LBL_qenc32insts->SetLabel("");

	LBL_ienc64insts->SetLabel("");
	LBL_menc64insts->SetLabel("");
	LBL_aenc64insts->SetLabel("");
	LBL_fenc64insts->SetLabel("");
	LBL_denc64insts->SetLabel("");
	//LBL_qenc64insts->SetLabel("");

	LBL_cencq0insts->SetLabel("");
	LBL_cencq1insts->SetLabel("");
	LBL_cencq2insts->SetLabel("");

	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&encIDEenc::OnSelBitsBtnClick);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&encIDEenc::OngoBckBtnClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&encIDEenc::OnaddAlpBtnClick);
	Connect(wxID_ANY,wxEVT_SHOW,(wxObjectEventFunction)&encIDEenc::OnShow);

    CHK_ienc32insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);
	CHK_menc32insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);
	CHK_aenc32insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);
	CHK_fenc32insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);
	CHK_denc32insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);
	//CHK_qenc32insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);

	CHK_ienc64insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);
	CHK_menc64insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);
	CHK_aenc64insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);
	CHK_fenc64insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);
	CHK_denc64insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);
	//CHK_qenc64insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);

	CHK_cencq0insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);
	CHK_cencq1insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);
	CHK_cencq2insts->Bind(wxEVT_CHECKBOX, &encIDEenc::OnCheckBoxes,this);


	wxFont font = LBL_ienc32insts->GetFont();
    font.SetPointSize(14);
    font.SetWeight(wxFONTWEIGHT_BOLD);
    LBL_ienc32insts->SetFont(font);

    key_text = new wxTextCtrl(this, ID_key_text, "00000000000000000000000000000000", wxPoint(1050,2), wxSize(250,30), 0, wxDefaultValidator, _T("ID_key_text"));
    keyall_text = new wxTextCtrl(this, ID_keyall_text, "00000000000000000000000000000000", wxPoint(1050,800), wxSize(250,30), 0, wxDefaultValidator, _T("ID_keyall_text"));
}

encIDEenc::~encIDEenc()
{
	//(*Destroy(encIDEenc)
	//*)
}

std::vector<std::string> opcodes {
"beq",
"bne",
"blt",
"bge",
"bltu",
"bgeu",
"jalr",
"jal",
"lui",
"auipc",
"addi",
"slli",
"slti",
"sltiu",
"xori",
"srli",
"srai",
"ori",
"andi",
"add",
"sub",
"sll",
"slt",
"sltu",
"xor",
"srl",
"sra",
"or",
"and",
"addiw",
"slliw",
"srliw",
"sraiw",
"addw",
"subw",
"sllw",
"srlw",
"sraw",
"lb",
"lh",
"lw",
"ld",
"lbu",
"lhu",
"lwu",
"sb",
"sh",
"sw",
"sd",
"fence",
"fence_i",
"mul",
"mulh",
"mulhsu",
"mulhu",
"div",
"divu",
"rem",
"remu",
"mulw",
"divw",
"divuw",
"remw",
"remuw",
"lr_w",
"sc_w",
"lr_d",
"sc_d",
"ecall",
"ebreak",
"uret",
"mret",
"dret",
"sfence_vma",
"wfi",
"csrrw",
"csrrs",
"csrrc",
"csrrwi",
"csrrsi",
"csrrci",
"slli_rv32",
"srli_rv32",
"srai_rv32",
"rdcycle",
"rdtime",
"rdinstret",
"rdcycleh",
"rdtimeh",
"rdinstreth"};



std::vector<std::string> opcodes_rv64i {
"lwu"
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
,"sraw"};

std::vector<std::string> opcodes_rv32m { // + rv64m
"mul"
,"mulh"
,"mulhsu"
,"mulhu"
,"div_"
,"divu"
,"rem"
,"remu"};

std::vector<std::string> opcodes_rv64m {
"mulw"
,"divw"
,"divuw"
,"remw"
,"remuw"};

std::vector<std::string> opcodes_rv32a { // + rv64a
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
,"amomaxu_w"};

std::vector<std::string> opcodes_rv64a {
"lr_d"
,"sc_d"
,"amoswap_d"
,"amoadd_d"
,"amoxor_d"
,"amoand_d"
,"amoor_d"
,"amomin_d"
,"amomax_d"
,"amominu_d"
,"amomaxu_d"};

std::vector<std::string> opcodes_rv32f { // + rv64d
"flw"
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
,"fmv_w_x"};

std::vector<std::string> opcodes_rv64f {
" fcvt_l_s"
," fcvt_lu_s"
," fcvt_s_l"
," fcvt_s_lu"};

std::vector<std::string> opcodes_rv32d {
"fld"
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
,"fcvt_d_wu"};

std::vector<std::string> opcodes_rv64d {
"fcvt.l.d"
,"fcvt.lu.d"
,"fmv.x.d"
,"fcvt.d.l"
,"fcvt.d.lu"
,"fmv.d.x"};

std::vector<std::string> opcodes_rv32c { // + rv64c
"c.addi4spn"
,"c.fld"
,"c.lw"
,"c.flw"
,"c.ld"
,"c.fsd"
,"c.sw"
,"c.fsw"
,"c.sd"
,"c.nop"
,"c.addi"
,"c.jal"
,"c.addiw"
,"c.li"
,"c.addi16sp"
,"c.lui"
,"c.srli"
,"c.srai"
,"c.andi"
,"c.sub"
,"c.xor"
,"c.or"
,"c.and"
,"c.subw"
,"c.addw"
,"c.j"
,"c.beqz"
,"c.bnez"
,"c.slli"
,"c.fldsp"
,"c.lwsp"
,"c.flwsp"
,"c.ldsp"
,"c.jr"
,"c.mv"
,"c.ebreak"
,"c.jalr"
,"c.add"
,"c.fsdsp"
,"c.swsp"
,"c.fswsp"
,"c.sdsp"};

std::vector<std::string> opcodes_rv32q {
"fsq"
,"fmadd.q"
,"fmsub.q"
,"fnmsub.q"
,"fnmadd.q"
,"fadd.q"
,"fsub.q"
,"fmul.q"
,"fdiv.q"
,"fsqrt.q"
,"fsgnj.q"
,"fsgnjn.q"
,"fsgnjx.q"
,"fmin.q"
,"fmax.q"
,"fcvt.s.q"
,"fcvt.q.s"
,"fcvt.d.q"
,"fcvt.q.d"
,"feq.q"
,"flt.q"
,"fle.q"
,"fclass.q"
,"fcvt.w.q"
,"fcvt.wu.q"
,"fcvt.q.w"
,"fcvt.q.w"};

std::vector<std::string> opcodes_rv64q {
"fcvt.l.q"
,"fcvt.lu.q"
,"fcvt.q.l"
,"fcvt.q.lu"};

std::vector<std::string> opcodes_rvcq0 {
"c_addi4spn"
,"c_fld"
//,"c_lq"
,"c_lw"
,"c_flw"
,"c_ld"
,"c_fsd"
//,"c_sq"
,"c_sw"
,"c_fsw"
,"c_sd"};

std::vector<std::string> opcodes_rvcq1{
"c_nop"
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
,"c_bnez"};

std::vector<std::string> opcodes_rvcq2{
"c_slli"
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
,"c_sdsp"};

std::vector<std::string> empty_vector{};

void vectorAdd(std::vector<std::string> &vector1, std::vector<std::string> &vector2)
{
    vector1.insert( vector1.end(), vector2.begin(), vector2.end() );
}

void vectorRemove(std::vector<std::string> &vector1, std::vector<std::string> &vector2)
{
vector1.erase( remove_if( begin(vector1),end(vector1),
    [&](auto x){return find(begin(vector2),end(vector2),x)!=end(vector2);}), end(vector1) );
}

void encIDEenc::OngoBckBtnClick(wxCommandEvent& event)
{
    wxGetApp().compStr += " --elf2encryptedhex=\" " + wxGetApp().alpStr + "\" ";
    wxGetApp().GetTopWindow()->Show();

    this->Close();
}

void CreateDynamicButtons(encIDEenc *thisWindow, std::vector<std::string> opcodes){
    int left = 10;
    int top = 60;
    int width = 96;
    int height = 32;

    wxButton* btns[opcodes.size()];
    for(wxInt8 i=0; i<opcodes.size(); i++){
        if(left > thisWindow->GetSize().GetWidth() - width){
            left = 10;
            top += 40;
        }
        btns[i] = new wxButton(thisWindow, 1000 + i, opcodes[i], wxPoint(left, top) /*wxPoint(150 + 5*i,150 + 5*i)*/, wxSize(width, height) /*wxDefaultSize*/, 0, wxDefaultValidator, opcodes[i]);
        btns[i]->Bind(wxEVT_BUTTON, &encIDEenc::OnButtons, thisWindow);
        btns[i]->SetBackgroundColour(*wxRED);
        left += width + 5;
    }
}

void RemoveAllDynamicButtons(encIDEenc *thisWindow, std::vector<std::string> opcodes){
    for(wxInt8 i=0; i<opcodes.size(); i++){
        if(wxWindow* ctrl = wxWindow::FindWindowById(1000 + i,thisWindow)){
            ctrl->Destroy();
        }
    }
}

void encIDEenc::OnShow(wxShowEvent & event)
{
    CreateDynamicButtons(this, opcodes_all);
}

void encIDEenc::OnButtons(wxCommandEvent & event)
{
    wxButton *btn = wxDynamicCast( event.GetEventObject(), wxButton);

    wxString lbl = LBL_ienc32insts->GetLabel();

    auto index = btn->GetId() - 1000;

    if(lbl[index].GetValue() == '0'){
        lbl.replace(index,1,"1");
        btn->SetBackgroundColour(*wxGREEN);
    }
    else{
        lbl.replace(index,1,"0");
        btn->SetBackgroundColour(*wxRED);
    }
    LBL_ienc32insts->SetLabel(lbl);
}


void encIDEenc::OnCheckBoxes(wxCommandEvent & event)
{
    wxCheckBox *chk = wxDynamicCast( event.GetEventObject(), wxCheckBox);

RemoveAllDynamicButtons(this, opcodes_all);
    if(chk->GetValue()==true){
        if(chk->GetId() == ID_CHK_ienc32insts){
            CHK_menc32insts->SetValue(false);
            CHK_aenc32insts->SetValue(false);
            CHK_fenc32insts->SetValue(false);
            CHK_denc32insts->SetValue(false);

            CHK_ienc64insts->SetValue(false);
            CHK_menc64insts->SetValue(false);
            CHK_aenc64insts->SetValue(false);
            CHK_fenc64insts->SetValue(false);
            CHK_denc64insts->SetValue(false);

            CHK_cencq0insts->SetValue(false);
            CHK_cencq1insts->SetValue(false);
            CHK_cencq2insts->SetValue(false);
            opcodes_all = opcodes_rv32i;
        }
        else if(chk->GetId() == ID_CHK_menc32insts){
            CHK_ienc32insts->SetValue(false);
            CHK_aenc32insts->SetValue(false);
            CHK_fenc32insts->SetValue(false);
            CHK_denc32insts->SetValue(false);

            CHK_ienc64insts->SetValue(false);
            CHK_menc64insts->SetValue(false);
            CHK_aenc64insts->SetValue(false);
            CHK_fenc64insts->SetValue(false);
            CHK_denc64insts->SetValue(false);

            CHK_cencq0insts->SetValue(false);
            CHK_cencq1insts->SetValue(false);
            CHK_cencq2insts->SetValue(false);
            opcodes_all = opcodes_rv32m;
        }
        else if(chk->GetId() == ID_CHK_aenc32insts){
            CHK_ienc32insts->SetValue(false);
            CHK_menc32insts->SetValue(false);
            CHK_fenc32insts->SetValue(false);
            CHK_denc32insts->SetValue(false);

            CHK_ienc64insts->SetValue(false);
            CHK_menc64insts->SetValue(false);
            CHK_aenc64insts->SetValue(false);
            CHK_fenc64insts->SetValue(false);
            CHK_denc64insts->SetValue(false);

            CHK_cencq0insts->SetValue(false);
            CHK_cencq1insts->SetValue(false);
            CHK_cencq2insts->SetValue(false);
            opcodes_all = opcodes_rv32a;
        }
        else if(chk->GetId() == ID_CHK_fenc32insts){
            CHK_ienc32insts->SetValue(false);
            CHK_menc32insts->SetValue(false);
            CHK_aenc32insts->SetValue(false);
            CHK_denc32insts->SetValue(false);

            CHK_ienc64insts->SetValue(false);
            CHK_menc64insts->SetValue(false);
            CHK_aenc64insts->SetValue(false);
            CHK_fenc64insts->SetValue(false);
            CHK_denc64insts->SetValue(false);

            CHK_cencq0insts->SetValue(false);
            CHK_cencq1insts->SetValue(false);
            CHK_cencq2insts->SetValue(false);
            opcodes_all = opcodes_rv32f;
        }
        else if(chk->GetId() == ID_CHK_denc32insts){
            CHK_ienc32insts->SetValue(false);
            CHK_menc32insts->SetValue(false);
            CHK_aenc32insts->SetValue(false);
            CHK_fenc32insts->SetValue(false);

            CHK_ienc64insts->SetValue(false);
            CHK_menc64insts->SetValue(false);
            CHK_aenc64insts->SetValue(false);
            CHK_fenc64insts->SetValue(false);
            CHK_denc64insts->SetValue(false);

            CHK_cencq0insts->SetValue(false);
            CHK_cencq1insts->SetValue(false);
            CHK_cencq2insts->SetValue(false);
            opcodes_all = opcodes_rv32d;
        }
        else if(chk->GetId() == ID_CHK_ienc64insts){
            CHK_ienc32insts->SetValue(false);
            CHK_menc32insts->SetValue(false);
            CHK_aenc32insts->SetValue(false);
            CHK_fenc32insts->SetValue(false);
            CHK_denc32insts->SetValue(false);


            CHK_menc64insts->SetValue(false);
            CHK_aenc64insts->SetValue(false);
            CHK_fenc64insts->SetValue(false);
            CHK_denc64insts->SetValue(false);

            CHK_cencq0insts->SetValue(false);
            CHK_cencq1insts->SetValue(false);
            CHK_cencq2insts->SetValue(false);
            opcodes_all = opcodes_rv64i;
        }
        else if(chk->GetId() == ID_CHK_menc64insts){
            CHK_ienc32insts->SetValue(false);
            CHK_menc32insts->SetValue(false);
            CHK_aenc32insts->SetValue(false);
            CHK_fenc32insts->SetValue(false);
            CHK_denc32insts->SetValue(false);

            CHK_ienc64insts->SetValue(false);
            CHK_aenc64insts->SetValue(false);
            CHK_fenc64insts->SetValue(false);
            CHK_denc64insts->SetValue(false);

            CHK_cencq0insts->SetValue(false);
            CHK_cencq1insts->SetValue(false);
            CHK_cencq2insts->SetValue(false);
            opcodes_all = opcodes_rv64m;
        }
        else if(chk->GetId() == ID_CHK_aenc64insts){
            CHK_ienc32insts->SetValue(false);
            CHK_menc32insts->SetValue(false);
            CHK_aenc32insts->SetValue(false);
            CHK_fenc32insts->SetValue(false);
            CHK_denc32insts->SetValue(false);

            CHK_ienc64insts->SetValue(false);
            CHK_menc64insts->SetValue(false);
            CHK_fenc64insts->SetValue(false);
            CHK_denc64insts->SetValue(false);

            CHK_cencq0insts->SetValue(false);
            CHK_cencq1insts->SetValue(false);
            CHK_cencq2insts->SetValue(false);
            opcodes_all = opcodes_rv64a;
        }
        else if(chk->GetId() == ID_CHK_fenc64insts){
            CHK_ienc32insts->SetValue(false);
            CHK_menc32insts->SetValue(false);
            CHK_aenc32insts->SetValue(false);
            CHK_fenc32insts->SetValue(false);
            CHK_denc32insts->SetValue(false);

            CHK_ienc64insts->SetValue(false);
            CHK_menc64insts->SetValue(false);
            CHK_aenc64insts->SetValue(false);
            CHK_denc64insts->SetValue(false);

            CHK_cencq0insts->SetValue(false);
            CHK_cencq1insts->SetValue(false);
            CHK_cencq2insts->SetValue(false);
            opcodes_all = opcodes_rv64f;
        }
        else if(chk->GetId() == ID_CHK_denc64insts){
            CHK_ienc32insts->SetValue(false);
            CHK_menc32insts->SetValue(false);
            CHK_aenc32insts->SetValue(false);
            CHK_fenc32insts->SetValue(false);
            CHK_denc32insts->SetValue(false);

            CHK_ienc64insts->SetValue(false);
            CHK_menc64insts->SetValue(false);
            CHK_aenc64insts->SetValue(false);
            CHK_fenc64insts->SetValue(false);

            CHK_cencq0insts->SetValue(false);
            CHK_cencq1insts->SetValue(false);
            CHK_cencq2insts->SetValue(false);
            opcodes_all = opcodes_rv64d;
        }
        else if(chk->GetId() == ID_CHK_cencq0insts){
            CHK_ienc32insts->SetValue(false);
            CHK_menc32insts->SetValue(false);
            CHK_aenc32insts->SetValue(false);
            CHK_fenc32insts->SetValue(false);
            CHK_denc32insts->SetValue(false);

            CHK_ienc64insts->SetValue(false);
            CHK_menc64insts->SetValue(false);
            CHK_aenc64insts->SetValue(false);
            CHK_fenc64insts->SetValue(false);
            CHK_denc64insts->SetValue(false);

            CHK_cencq1insts->SetValue(false);
            CHK_cencq2insts->SetValue(false);
            opcodes_all = opcodes_rvcq0;
        }
        else if(chk->GetId() == ID_CHK_cencq1insts){
            CHK_ienc32insts->SetValue(false);
            CHK_menc32insts->SetValue(false);
            CHK_aenc32insts->SetValue(false);
            CHK_fenc32insts->SetValue(false);
            CHK_denc32insts->SetValue(false);

            CHK_ienc64insts->SetValue(false);
            CHK_menc64insts->SetValue(false);
            CHK_aenc64insts->SetValue(false);
            CHK_fenc64insts->SetValue(false);
            CHK_denc64insts->SetValue(false);

            CHK_cencq0insts->SetValue(false);
            CHK_cencq2insts->SetValue(false);
            opcodes_all = opcodes_rvcq1;
        }
        else if(chk->GetId() == ID_CHK_cencq2insts){
            CHK_ienc32insts->SetValue(false);
            CHK_menc32insts->SetValue(false);
            CHK_aenc32insts->SetValue(false);
            CHK_fenc32insts->SetValue(false);
            CHK_denc32insts->SetValue(false);

            CHK_ienc64insts->SetValue(false);
            CHK_menc64insts->SetValue(false);
            CHK_aenc64insts->SetValue(false);
            CHK_fenc64insts->SetValue(false);
            CHK_denc64insts->SetValue(false);

            CHK_cencq0insts->SetValue(false);
            CHK_cencq1insts->SetValue(false);
            opcodes_all = opcodes_rvcq2;
        }

        LBL_ienc32insts->SetLabel(std::string(opcodes_all.size(), '0'));
    }


    if(chk->GetValue()==true){
        CreateDynamicButtons(this, opcodes_all);
    }

    else{
        RemoveAllDynamicButtons(this, opcodes_all);
        opcodes_all = {};
    }
}

void encIDEenc::OnSelBitsBtnClick(wxCommandEvent& event)
{
    encIDEselBits* Frame = new encIDEselBits(0);
    Frame->CenterOnParent();
    Frame->Show();
    this->Close();
}

std::string addflagrv32i = "";
std::string addflagrv32m = "";
std::string addflagrv32a = "";
std::string addflagrv32f = "";
std::string addflagrv32d = "";

std::string addflagrv64i = "";
std::string addflagrv64m = "";
std::string addflagrv64a = "";
std::string addflagrv64f = "";
std::string addflagrv64d = "";

std::string addflagrvcq0 = "";
std::string addflagrvcq1 = "";
std::string addflagrvcq2 = "";

std::string addflagtext = "";

void encIDEenc::OnaddAlpBtnClick(wxCommandEvent& event)
{
    if(LBL_ienc32insts->GetLabelText() != std::string(opcodes_all.size(), '0') ){

        if(CHK_ienc32insts->GetValue()==true)
            addflagrv32i = " --ienc32insts=" + LBL_ienc32insts->GetLabelText() + " --ienc32key=" + key_text->GetValue();
        else if(CHK_menc32insts->GetValue()==true)
            addflagrv32m = " --menc32insts=" + LBL_ienc32insts->GetLabelText() + " --menc32key=" + key_text->GetValue();
        else if(CHK_aenc32insts->GetValue()==true)
            addflagrv32a = " --aenc32insts=" + LBL_ienc32insts->GetLabelText() + " --aenc32key=" + key_text->GetValue();
        else if(CHK_fenc32insts->GetValue()==true)
            addflagrv32f = " --fenc32insts=" + LBL_ienc32insts->GetLabelText() + " --fenc32key=" + key_text->GetValue();
        else if(CHK_denc32insts->GetValue()==true)
            addflagrv32d = " --denc32insts=" + LBL_ienc32insts->GetLabelText() + " --denc32key=" + key_text->GetValue();

        else if(CHK_ienc64insts->GetValue()==true)
            addflagrv64i = " --ienc64insts=" + LBL_ienc32insts->GetLabelText() + " --ienc64key=" + key_text->GetValue();
        else if(CHK_menc64insts->GetValue()==true)
            addflagrv64m = " --menc64insts=" + LBL_ienc32insts->GetLabelText() + " --menc64key=" + key_text->GetValue();
        else if(CHK_aenc64insts->GetValue()==true)
            addflagrv64a = " --aenc64insts=" + LBL_ienc32insts->GetLabelText() + " --aenc64key=" + key_text->GetValue();
        else if(CHK_fenc64insts->GetValue()==true)
            addflagrv64f = " --fenc64insts=" + LBL_ienc32insts->GetLabelText() + " --fenc64key=" + key_text->GetValue();
        else if(CHK_denc64insts->GetValue()==true)
            addflagrv64d = " --denc64insts=" + LBL_ienc32insts->GetLabelText() + " --denc64key=" + key_text->GetValue();

        else if(CHK_cencq0insts->GetValue()==true)
            addflagrvcq0 = " --cencq0insts=" + LBL_ienc32insts->GetLabelText() + " --cencq0key=" + key_text->GetValue();
        else if(CHK_cencq1insts->GetValue()==true)
            addflagrvcq1 = " --cencq1insts=" + LBL_ienc32insts->GetLabelText() + " --cencq1key=" + key_text->GetValue();
        else if(CHK_cencq2insts->GetValue()==true)
            addflagrvcq2 = " --cencq2insts=" + LBL_ienc32insts->GetLabelText() + " --cencq2key=" + key_text->GetValue();

        wxGetApp().alpStr = " --enckeyall=" + keyall_text->GetValue() + addflagrv32i + addflagrv32m + addflagrv32a + addflagrv32f + addflagrv32d + addflagrv64i + addflagrv64m + addflagrv64a + addflagrv64f + addflagrv64d + addflagrvcq0 + addflagrvcq1 + addflagrvcq2;

        wxMessageBox(wxGetApp().alpStr + " flags added.");
    }
    else wxGetApp().alpStr = "";
}
