#include "kasirgaIDEenc.h"
#include "kasirgaIDEMain.h"

#include "kasirgaIDEApp.h"
//#include "kasirgaIDEMain.cpp"

#include <wx/msgdlg.h>


//(*InternalHeaders(kasirgaIDEenc)
#include <wx/string.h>
#include <wx/intl.h>
//*)

//(*IdInit(kasirgaIDEenc)

//*)

const long kasirgaIDEenc::ID_CHECKBOX1 = wxNewId();
const long kasirgaIDEenc::ID_CHECKBOX2 = wxNewId();
const long kasirgaIDEenc::ID_CHECKBOX3 = wxNewId();
const long kasirgaIDEenc::ID_CHECKBOX4 = wxNewId();
const long kasirgaIDEenc::ID_CHECKBOX5 = wxNewId();

const long kasirgaIDEenc::ID_BUTTON1 = wxNewId();
const long kasirgaIDEenc::ID_BUTTON2 = wxNewId();
const long kasirgaIDEenc::ID_BUTTON3 = wxNewId();
//const long kasirgaIDEenc::ID_STATICTEXT1 = wxNewId();
const long kasirgaIDEenc::ID_LBL_ienc32insts = wxNewId();
const long kasirgaIDEenc::ID_LBL_menc32insts = wxNewId();
const long kasirgaIDEenc::ID_LBL_aenc32insts = wxNewId();
const long kasirgaIDEenc::ID_LBL_fenc32insts = wxNewId();
const long kasirgaIDEenc::ID_LBL_denc32insts = wxNewId();
const long kasirgaIDEenc::ID_LBL_qenc32insts = wxNewId();

const long kasirgaIDEenc::ID_LBL_ienc64insts = wxNewId();
const long kasirgaIDEenc::ID_LBL_menc64insts = wxNewId();
const long kasirgaIDEenc::ID_LBL_aenc64insts = wxNewId();
const long kasirgaIDEenc::ID_LBL_fenc64insts = wxNewId();
const long kasirgaIDEenc::ID_LBL_denc64insts = wxNewId();
const long kasirgaIDEenc::ID_LBL_qenc64insts = wxNewId();

const long kasirgaIDEenc::ID_LBL_cencq2insts = wxNewId();
const long kasirgaIDEenc::ID_LBL_cencq1insts = wxNewId();
const long kasirgaIDEenc::ID_LBL_cencq0insts = wxNewId();


BEGIN_EVENT_TABLE(kasirgaIDEenc,wxFrame)
	//(*EventTable(kasirgaIDEenc)
	//*)
	//EVT_BUTTON(kasirgaIDEenc::OnButtons)
END_EVENT_TABLE()


kasirgaIDEenc::kasirgaIDEenc(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(kasirgaIDEenc)
	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
	SetClientSize(wxSize(834,450));

	//*)

    //kasirgaIDEFrame *mainFrame;
    //SetMainFrame(kasirgaIDEFrame *aFrame) { mainFrame = aFrame;}

    CheckBox1 = new wxCheckBox(this, ID_CHECKBOX1, _("rv32i"), wxPoint(264,4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox1->SetValue(false);
	CheckBox2 = new wxCheckBox(this, ID_CHECKBOX2, _("rv32m"), wxPoint(344,4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox2->SetValue(false);
	CheckBox3 = new wxCheckBox(this, ID_CHECKBOX3, _("rv32a"), wxPoint(416,4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox3->SetValue(false);
	CheckBox4 = new wxCheckBox(this, ID_CHECKBOX4, _("rv32f"), wxPoint(488,4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	CheckBox4->SetValue(false);
	CheckBox5 = new wxCheckBox(this, ID_CHECKBOX5, _("rv32c"), wxPoint(560,4), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
	CheckBox5->SetValue(false);

    SetClientSize(wxSize(968,478));
	goBckBtn = new wxButton(this, ID_BUTTON1, _("Go back"), wxPoint(8,8), wxSize(88,29), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button1 = new wxButton(this, ID_BUTTON2, _("Select bits"), wxPoint(104,8), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    addAlp = new wxButton(this, ID_BUTTON3, _("Add flag"), wxPoint(826,2), wxSize(88,29), 0, wxDefaultValidator, _T("ID_BUTTON3"));

    //lbl1 = new wxStaticText(this, ID_STATICTEXT1, _("000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"), wxPoint(192,32), wxSize(768,32), 0, _T("ID_STATICTEXT1"));

    LBL_ienc32insts = new wxStaticText(this, ID_LBL_ienc32insts, std::string(47, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_ienc32insts"));
	LBL_menc32insts = new wxStaticText(this, ID_LBL_menc32insts, std::string(8 , '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_menc32insts"));
	LBL_aenc32insts = new wxStaticText(this, ID_LBL_aenc32insts, std::string(11, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_aenc32insts"));
	LBL_fenc32insts = new wxStaticText(this, ID_LBL_fenc32insts, std::string(26, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_fenc32insts"));
	LBL_denc32insts = new wxStaticText(this, ID_LBL_denc32insts, std::string(26, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_denc32insts"));
	LBL_qenc32insts = new wxStaticText(this, ID_LBL_qenc32insts, std::string(28, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_qenc32insts"));

	LBL_ienc64insts = new wxStaticText(this, ID_LBL_ienc64insts, std::string(12, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_ienc64insts"));
	LBL_menc64insts = new wxStaticText(this, ID_LBL_menc64insts, std::string(5 , '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_menc64insts"));
	LBL_aenc64insts = new wxStaticText(this, ID_LBL_aenc64insts, std::string(11, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_aenc64insts"));
	LBL_fenc64insts = new wxStaticText(this, ID_LBL_fenc64insts, std::string(4 , '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_fenc64insts"));
	LBL_denc64insts = new wxStaticText(this, ID_LBL_denc64insts, std::string(6 , '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_denc64insts"));
	LBL_qenc64insts = new wxStaticText(this, ID_LBL_qenc64insts, std::string(4 , '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_qenc64insts"));

	LBL_cencq0insts = new wxStaticText(this, ID_LBL_cencq0insts, std::string(11, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_cencq0insts"));
	LBL_cencq1insts = new wxStaticText(this, ID_LBL_cencq1insts, std::string(21, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_cencq1insts"));
	LBL_cencq2insts = new wxStaticText(this, ID_LBL_cencq2insts, std::string(17, '0'), wxPoint(192,32), wxSize(768,32), 0, _T("ID_LBL_cencq2insts"));

	LBL_ienc64insts->Show(false);
	LBL_menc64insts->Show(false);
	LBL_aenc64insts->Show(false);
	LBL_fenc64insts->Show(false);
	LBL_denc64insts->Show(false);
	LBL_qenc64insts->Show(false);

	LBL_ienc64insts->Show(false);
	LBL_menc64insts->Show(false);
	LBL_aenc64insts->Show(false);
	LBL_fenc64insts->Show(false);
	LBL_denc64insts->Show(false);
	LBL_qenc64insts->Show(false);

	LBL_cencq0insts->Show(false);
	LBL_cencq1insts->Show(false);
	LBL_cencq2insts->Show(false);

	//Layout();

	CheckBox1->SetValue(true);

	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&kasirgaIDEenc::OnSelBitsBtnClick);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&kasirgaIDEenc::OngoBckBtnClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&kasirgaIDEenc::OnaddAlpBtnClick);
	Connect(wxID_ANY,wxEVT_SHOW,(wxObjectEventFunction)&kasirgaIDEenc::OnShow);

	//Connect(ID_STATICTEXT1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&kasirgaIDEenc::OnButtons);
	//Connect(ID_STATICTEXT1,wxEVT_BUTTON,(wxObjectEventFunction)&kasirgaIDEenc::OnButtons);

	//CheckBox1->Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&kasirgaIDEenc::OnCheckBoxes);
	//CheckBox2->Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&kasirgaIDEenc::OnCheckBoxes);
	//CheckBox3->Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&kasirgaIDEenc::OnCheckBoxes);
	//CheckBox4->Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&kasirgaIDEenc::OnCheckBoxes);
	//CheckBox5->Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&kasirgaIDEenc::OnCheckBoxes);

    //CheckBox1->Connect(ID_CHECKBOX1,wxEVT_CHECKBOX,(wxObjectEventFunction)&kasirgaIDEenc::OnCheckBoxes);
	//CheckBox2->Connect(ID_CHECKBOX2,wxEVT_CHECKBOX,(wxObjectEventFunction)&kasirgaIDEenc::OnCheckBoxes);
	//CheckBox3->Connect(ID_CHECKBOX2,wxEVT_CHECKBOX,(wxObjectEventFunction)&kasirgaIDEenc::OnCheckBoxes);
	//CheckBox4->Connect(ID_CHECKBOX2,wxEVT_CHECKBOX,(wxObjectEventFunction)&kasirgaIDEenc::OnCheckBoxes);
	//CheckBox5->Connect(ID_CHECKBOX2,wxEVT_CHECKBOX,(wxObjectEventFunction)&kasirgaIDEenc::OnCheckBoxes);

	CheckBox1->Bind(wxEVT_CHECKBOX, &kasirgaIDEenc::OnCheckBoxes, this);
	CheckBox2->Bind(wxEVT_CHECKBOX, &kasirgaIDEenc::OnCheckBoxes, this);
	CheckBox3->Bind(wxEVT_CHECKBOX, &kasirgaIDEenc::OnCheckBoxes, this);
	CheckBox4->Bind(wxEVT_CHECKBOX, &kasirgaIDEenc::OnCheckBoxes, this);
	CheckBox5->Bind(wxEVT_CHECKBOX, &kasirgaIDEenc::OnCheckBoxes, this);



	//Connect(wxID_ANY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&kasirgaIDEenc::OnButtons);
}

kasirgaIDEenc::~kasirgaIDEenc()
{
	//(*Destroy(kasirgaIDEenc)
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

std::vector<std::string> opcodes_rv32i { // + rv64i
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
,"bgeu"};

std::vector<std::string> opcodes_rv64i {
"addiw"
,"slliw"
,"srliw"
,"sraiw"
,"addw"
,"subw"
,"sllw"
,"srlw"
,"sraw"
,"lwu"
,"ld"
,"sd"};

std::vector<std::string> opcodes_rv32m { // + rv64m
"mul"
,"mulh"
,"mulhsu"
,"mulhu"
,"div"
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
"lr.w"
,"sc.w"
,"amoswap.w"
,"amoadd.w"
,"amoxor.w"
,"amoand.w"
,"amoor.w"
,"amomin.w"
,"amomax.w"
,"amominu.w"
,"amomaxu.w"};

std::vector<std::string> opcodes_rv64a {
"lr.d"
,"sc.d"
,"amoswap.d"
,"amoadd.d"
,"amoxor.d"
,"amoand.d"
,"amoor.d"
,"amomin.d"
,"amomax.d"
,"amominu.d"
,"amomaxu.d"};

std::vector<std::string> opcodes_rv32f { // + rv64d
"fmadd.s"
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
,"fsd"};

std::vector<std::string> opcodes_rv64f {
"fcvt.l.s"
,"fcvt.lu.s"
,"fcvt.s.l"
,"fcvt.s.lu"};

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


std::vector<std::string> opcodes_all{};

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

void kasirgaIDEenc::OngoBckBtnClick(wxCommandEvent& event)
{
    //wxApp2frame2* Frame = new wxApp2frame2(0);
    //wxMessageBox(opcodes[1]);

    //kasirgaIDEFrame* Frame = new kasirgaIDEFrame(0);//(this, wxID_ANY);
    //Frame->CenterOnParent();
    //Frame->Show();
    //kasirgaIDEApp::GetTopWindow()->Show();
    //kasirgaIDEFrame->Show();
    //kasirgaIDEApp::wxGetApp().kasirgaIDEFrame->Show();
    //wxGetApp().Frame->Show();
    //wxGetApp().compStr = "";//wxT("abcd").ToStdString();

    wxGetApp().compStr += " --alp=\" " + wxGetApp().alpStr + "\" ";
    wxGetApp().GetTopWindow()->Show();
    //wxMessageBox(wxGetApp().compStr);

    //SetTopWindow(Frame);
    this->Close();
}

void CreateDynamicButtons(kasirgaIDEenc *thisWindow, std::vector<std::string> opcodes){
    int left = 10;
    int top = 50;
    int width = 96; //70;
    int height = 32; //40;
/*
    int i;
    wxButton* button[2];
    for(i=0;i<2;i++)
    {
        button[i] = new wxButton(this,1000+i,wxString::Format(_("%i"),i),wxPoint(0,i*30));
        Connect(1000+i, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyFrame::OnButton));
    }
*/
    wxButton* btns[opcodes.size()];
    for(wxInt8 i=0; i<opcodes.size(); i++){
        if(left > thisWindow->GetSize().GetWidth() - width){
            left = 10;
            top += 40;
            //wxMessageBox("" + top);
        }
        //wxID_HIGHEST
        btns[i] = new wxButton(thisWindow, 1000 + i, opcodes[i], wxPoint(left, top) /*wxPoint(150 + 5*i,150 + 5*i)*/, wxSize(width, height) /*wxDefaultSize*/, 0, wxDefaultValidator, opcodes[i]);
        btns[i]->Bind(wxEVT_BUTTON, &kasirgaIDEenc::OnButtons, thisWindow);
        btns[i]->SetBackgroundColour(*wxRED);
        //myButton->Connect(wxEVT_BUTTON, wxCommandEventHandler(MyFrame::OnMyButtonClicked), NULL, this);
        //btns[i]->Connect(1000 + i, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(kasirgaIDEenc::OnButtons));
        left += width + 5;
        //btns[i]->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &kasirgaIDEenc::OnButtons, thisWindow, wxID_EXIT);
    }
}

void RemoveAllDynamicButtons(kasirgaIDEenc *thisWindow, std::vector<std::string> opcodes){
    for(wxInt8 i=0; i<opcodes.size(); i++){
        if(wxWindow* ctrl = wxWindow::FindWindowById(1000 + i,thisWindow)){
            //ctrl->Unbind(wxEVT_BUTTON, &kasirgaIDEenc::OnButtons, thisWindow);
            ctrl->Destroy();
        }
    }
}

void kasirgaIDEenc::OnShow(wxShowEvent & event)
{
    //opcodes_all = opcodes;
    //opcodes_all.erase( remove_if( begin(opcodes_all),end(opcodes_all),
    //[&](auto x){return find(begin(opcodes_rv32i),end(opcodes_rv32i),x)!=end(opcodes_rv32i);}), end(opcodes_all) );

    CreateDynamicButtons(this, opcodes_all);
}

void kasirgaIDEenc::OnButtons(wxCommandEvent & event)
{
    /*
    switch(event.GetId())
    {
        case wxID_BUTTON1:
            DoSomething();
            break;
        case wxID_BUTTON2:
            DoSomethingElse();*
            break;
        ....
    }
    */

    //wxButton* btn = dynamic_cast<wxButton*> (event.GetEventObject());
    wxButton *btn = wxDynamicCast( event.GetEventObject(), wxButton);
    //btn->SetBackgroundColour(*wxRED);
    //static bool click;
    //if(click){
    //click = false;
    //btn->SetBackgroundColour(*wxRED);
    //}
    //else
    //{
    //click = true;
    //btn->SetBackgroundColour(*wxGREEN);
    //}

    wxString lbl = LBL_aenc32insts->GetLabel();
    //wxMessageBox(lbl);
    //lbl1->SetLabel("100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");

    //auto index = 0;
    //auto it = std::find(opcodes.begin(), opcodes.end(), btn->GetLabelText().ToStdString());
    //if (it != opcodes.end())
    //{
    //index = std::distance(opcodes.begin(), it);
    //}

    auto index = btn->GetId() - 1000;

//lbl.replace(index,1,'0');
    //std::string lbl = lbl1->GetLabel().ToStdString();
    //wxMessageBox();
    //if(lbl1->GetLabel().Find("0") != wxNOT_FOUND)
    //wxString lbl(lbl1->GetLabel());
    //wxString str = lbl1->GetLabel();
    //wxMessageBox(lbl1->GetLabel());

    if(lbl[index].GetValue() == '0'){
        lbl.replace(index,1,"1");
        btn->SetBackgroundColour(*wxGREEN);
    }
    else{
        lbl.replace(index,1,"0");
        btn->SetBackgroundColour(*wxRED);
    }
    LBL_aenc32insts->SetLabel(lbl);

    //wxMessageBox(wxString::Format(wxT("%i"),btn->GetId()));

    //idsini vererek yeni wxStaticText* oluşturmayi dene.
}


void kasirgaIDEenc::OnCheckBoxes(wxCommandEvent & event)
{
    wxCheckBox *chk = wxDynamicCast( event.GetEventObject(), wxCheckBox);


    //wxButton* btns[opcodes.size()];

    //for(wxInt8 i=0; i<opcodes.size(); i++){
    //    btns[i]->Unbind(wxEVT_BUTTON, &kasirgaIDEenc::OnButtons, this);
    //}
RemoveAllDynamicButtons(this, opcodes_all);
    if(chk->GetValue()==true){
        if(chk->GetId() == ID_CHECKBOX1){
            CheckBox2->SetValue(false);
            CheckBox3->SetValue(false);
            CheckBox4->SetValue(false);
            CheckBox5->SetValue(false);
            opcodes_all = opcodes_rv32i;
        }
        else if(chk->GetId() == ID_CHECKBOX2){
            CheckBox1->SetValue(false);
            CheckBox3->SetValue(false);
            CheckBox4->SetValue(false);
            CheckBox5->SetValue(false);
            opcodes_all = opcodes_rv32m;
        }
        else if(chk->GetId() == ID_CHECKBOX3){
            CheckBox1->SetValue(false);
            CheckBox2->SetValue(false);
            CheckBox4->SetValue(false);
            CheckBox5->SetValue(false);
            opcodes_all = opcodes_rv32a;
        }
        else if(chk->GetId() == ID_CHECKBOX4){
            CheckBox1->SetValue(false);
            CheckBox2->SetValue(false);
            CheckBox3->SetValue(false);
            CheckBox5->SetValue(false);
            opcodes_all = opcodes_rv32f;
        }
        else if(chk->GetId() == ID_CHECKBOX5){
            CheckBox1->SetValue(false);
            CheckBox2->SetValue(false);
            CheckBox3->SetValue(false);
            CheckBox4->SetValue(false);
            opcodes_all = opcodes_rv32c;
        }
    }


    if(chk->GetValue()==true){

            /*
        switch(chk->GetId()){
            case ID_CHECKBOX1: opcodes_all = opcodes_rv32i; //vectorAdd(opcodes_all, opcodes_rv32i);
            break;
            case ID_CHECKBOX2: opcodes_all = opcodes_rv32m; //vectorAdd(opcodes_all, opcodes_rv32m);
            break;
            case ID_CHECKBOX3: opcodes_all = opcodes_rv32a; //vectorAdd(opcodes_all, opcodes_rv32a);
            break;
            case ID_CHECKBOX4: opcodes_all = opcodes_rv32f; //vectorAdd(opcodes_all, opcodes_rv32f);
            break;
            case ID_CHECKBOX5: opcodes_all = opcodes_rv32c; //vectorAdd(opcodes_all, opcodes_rv32c);
            break;
            //case ID_CHECKBOX1: vectorAdd(opcodes_all, opcodes_rv32i);
            //case ID_CHECKBOX2: vectorAdd(opcodes_all, opcodes_rv32m);
            //case ID_CHECKBOX3: vectorAdd(opcodes_all, opcodes_rv32a);
            //case ID_CHECKBOX4: vectorAdd(opcodes_all, opcodes_rv32f);
            //case ID_CHECKBOX5: vectorAdd(opcodes_all, opcodes_rv32c);
        }*/

        CreateDynamicButtons(this, opcodes_all);
    }

    else{
        RemoveAllDynamicButtons(this, opcodes_all);
        opcodes_all = {}; //empty_vector;
        /*
        switch(chk->GetId()){
            case 350: vectorRemove(opcodes_all, opcodes_rv32i);
            break;
            case 351: vectorRemove(opcodes_all, opcodes_rv32m);
            break;
            case 352: vectorRemove(opcodes_all, opcodes_rv32a);
            break;
            case 353: vectorRemove(opcodes_all, opcodes_rv32f);
            break;
            case 354: vectorRemove(opcodes_all, opcodes_rv32c);
            break;
        }*/

    }



}

void kasirgaIDEenc::OnSelBitsBtnClick(wxCommandEvent& event)
{
    kasirgaIDEselBits* Frame = new kasirgaIDEselBits(0);
    Frame->CenterOnParent();
    Frame->Show();
    this->Close();
}

void kasirgaIDEenc::OnaddAlpBtnClick(wxCommandEvent& event)
{
    if(LBL_aenc32insts->GetLabelText() != "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"){
        wxGetApp().alpStr = " --bits=" + LBL_aenc32insts->GetLabelText();
        wxMessageBox(wxGetApp().alpStr + " flag added.");
    }
    else wxGetApp().alpStr = "";
}
