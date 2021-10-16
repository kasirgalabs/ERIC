#include "kasirgaIDEselBits.h"

#include "kasirgaIDEApp.h"

#include <wx/msgdlg.h>

//(*InternalHeaders(kasirgaIDEselBits)
#include <wx/string.h>
#include <wx/intl.h>
//*)

//(*IdInit(kasirgaIDEselBits)
const long kasirgaIDEselBits::ID_CHOICE1 = wxNewId();
const long kasirgaIDEselBits::ID_STATICTEXT1 = wxNewId();
//*)

const long kasirgaIDEselBits::ID_BUTTON1 = wxNewId();
const long kasirgaIDEselBits::ID_BUTTON3 = wxNewId();
const long kasirgaIDEselBits::ID_STATICTEXT2 = wxNewId();

BEGIN_EVENT_TABLE(kasirgaIDEselBits,wxFrame)
	//(*EventTable(kasirgaIDEselBits)
	//*)
END_EVENT_TABLE()

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

kasirgaIDEselBits::kasirgaIDEselBits(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(kasirgaIDEselBits)
	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
	SetClientSize(wxSize(790,450));
	Choice1 = new wxChoice(this, ID_CHOICE1, wxPoint(136,64), wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Select Instruction:"), wxPoint(8,64), wxDefaultSize, 0, _T("ID_STATICTEXT1"));

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&kasirgaIDEselBits::OnChoice1Select);
	//*)

	goBckBtn = new wxButton(this, ID_BUTTON1, _("Go back"), wxPoint(8,8), wxSize(88,29), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&kasirgaIDEselBits::OngoBckBtnClick);

    lbl1 = new wxStaticText(this, ID_STATICTEXT2, _("00000000000000000000000000000000"), wxPoint(192,8), wxSize(768,32), 0, _T("ID_STATICTEXT2"));

    addAlp = new wxButton(this, ID_BUTTON3, _("Add flag"), wxPoint(688,8), wxSize(88,29), 0, wxDefaultValidator, _T("ID_BUTTON3"));
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&kasirgaIDEselBits::OnaddAlpBtnClick);

    for(wxInt8 i=0; i<opcodes_rv32.size(); i++){
        Choice1->Append(opcodes_rv32[i]);
    }

    int left = 10;
    int top = 100;
    int width = 24;
    int height = 24;

    wxButton* btns[32];
    for(wxInt8 i=0; i<32; i++){
        if(left > this->GetSize().GetWidth() - width){
            left = 10;
            top += 40;
        }

        //btns[i] = new wxButton(this, 1000 + i, wxString::Format(wxT("%i"), 1000 + i), wxPoint(left, top), wxSize(width, height), 0, wxDefaultValidator, wxString::Format(wxT("%i"), 1000 + i));
        btns[i] = new wxButton(this, 1000 + i, wxString::Format(wxT("%i"), 0), wxPoint(left, top), wxSize(width, height), 0, wxDefaultValidator, wxString::Format(wxT("%i"), 0));
        btns[i]->Bind(wxEVT_BUTTON, &kasirgaIDEselBits::OnBitButtons, this);
        btns[i]->SetBackgroundColour(*wxRED);
        left += width;
    }

}

kasirgaIDEselBits::~kasirgaIDEselBits()
{
	//(*Destroy(kasirgaIDEselBits)
	//*)
}



void kasirgaIDEselBits::OngoBckBtnClick(wxCommandEvent& event)
{
    kasirgaIDEenc* Frame = new kasirgaIDEenc(0);
    Frame->CenterOnParent();
    Frame->Show();
    this->Close();
}

/*
void CreateDynamicBitButtons(kasirgaIDEselBits *thisWindow){
    int left = 10;
    int top = 40;
    int width = 32;
    int height = 32;

    wxButton* btns[32];
    for(wxInt8 i=0; i<32; i++){
        if(left > thisWindow->GetSize().GetWidth() - width){
            left = 10;
            top += 40;
        }

        btns[i] = new wxButton(thisWindow, 1000 + i, wxString::Format(wxT("%i"), 1000 + i), wxPoint(left, top), wxSize(width, height), 0, wxDefaultValidator, wxString::Format(wxT("%i"), 1000 + i));
        btns[i]->Bind(wxEVT_BUTTON, &kasirgaIDEenc::OnBitButtons, thisWindow);
        left += width + 5;
    }
}
*/

void kasirgaIDEselBits::OnBitButtons(wxCommandEvent & event)
{

    wxButton *btn = wxDynamicCast( event.GetEventObject(), wxButton);

    wxString lbl = lbl1->GetLabel();

    auto index = btn->GetId() - 1000;

    if(lbl[index].GetValue() == '0'){
        lbl.replace(index,1,"1");
        btn->SetBackgroundColour(*wxGREEN);
        btn->SetLabel("1");
    }
    else{
        lbl.replace(index,1,"0");
        btn->SetBackgroundColour(*wxRED);
        btn->SetLabel("0");
    }
    lbl1->SetLabel(lbl);
}

std::string inst = "";

void kasirgaIDEselBits::OnChoice1Select(wxCommandEvent& event)
{
    //wxMessageBox(opcodes_rv32[Choice1->GetSelection()]);
    inst = opcodes_rv32[Choice1->GetSelection()];
}

void kasirgaIDEselBits::OnaddAlpBtnClick(wxCommandEvent& event)
{
    if(inst != ""){
        wxGetApp().alpStr += " --b_p_" + inst + "=" + lbl1->GetLabelText() + " ";
        wxMessageBox(wxGetApp().alpStr + " flags added.");
    }
}
