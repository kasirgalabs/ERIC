#include "encIDEselBits.h"

#include "encIDEApp.h"

#include <wx/msgdlg.h>

//(*InternalHeaders(encIDEselBits)
#include <wx/string.h>
#include <wx/intl.h>
//*)

//(*IdInit(encIDEselBits)
const long encIDEselBits::ID_CHOICE1 = wxNewId();
const long encIDEselBits::ID_STATICTEXT1 = wxNewId();
//*)

const long encIDEselBits::ID_BUTTON1 = wxNewId();
const long encIDEselBits::ID_BUTTON3 = wxNewId();
const long encIDEselBits::ID_STATICTEXT2 = wxNewId();

BEGIN_EVENT_TABLE(encIDEselBits,wxFrame)
	//(*EventTable(encIDEselBits)
	//*)
END_EVENT_TABLE()

std::vector<std::string> opcodes_alli{
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

encIDEselBits::encIDEselBits(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(encIDEselBits)
	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
	SetClientSize(wxSize(790,450));
	Choice1 = new wxChoice(this, ID_CHOICE1, wxPoint(136,64), wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Select Instruction:"), wxPoint(8,64), wxDefaultSize, 0, _T("ID_STATICTEXT1"));

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&encIDEselBits::OnChoice1Select);
	//*)

	goBckBtn = new wxButton(this, ID_BUTTON1, _("Go back"), wxPoint(8,8), wxSize(88,29), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&encIDEselBits::OngoBckBtnClick);

    lbl1 = new wxStaticText(this, ID_STATICTEXT2, _("00000000000000000000000000000000"), wxPoint(192,8), wxSize(768,32), 0, _T("ID_STATICTEXT2"));

    addAlp = new wxButton(this, ID_BUTTON3, _("Add flag"), wxPoint(688,8), wxSize(88,29), 0, wxDefaultValidator, _T("ID_BUTTON3"));
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&encIDEselBits::OnaddAlpBtnClick);

    for(wxInt8 i=0; i<opcodes_alli.size(); i++){
        Choice1->Append(opcodes_alli[i]);
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
        btns[i]->Bind(wxEVT_BUTTON, &encIDEselBits::OnBitButtons, this);
        btns[i]->SetBackgroundColour(*wxRED);
        left += width;
    }

}

encIDEselBits::~encIDEselBits()
{
	//(*Destroy(encIDEselBits)
	//*)
}



void encIDEselBits::OngoBckBtnClick(wxCommandEvent& event)
{
    encIDEenc* Frame = new encIDEenc(0);
    Frame->CenterOnParent();
    Frame->Show();
    this->Close();
}

/*
void CreateDynamicBitButtons(encIDEselBits *thisWindow){
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
        btns[i]->Bind(wxEVT_BUTTON, &encIDEenc::OnBitButtons, thisWindow);
        left += width + 5;
    }
}
*/

void encIDEselBits::OnBitButtons(wxCommandEvent & event)
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

void encIDEselBits::OnChoice1Select(wxCommandEvent& event)
{
    //wxMessageBox(opcodes_rv32[Choice1->GetSelection()]);
    inst = opcodes_alli[Choice1->GetSelection()];
}

void encIDEselBits::OnaddAlpBtnClick(wxCommandEvent& event)
{
    if(inst != ""){
        wxGetApp().alpStr += " --b_p_" + inst + "=" + lbl1->GetLabelText() + " ";
        wxMessageBox(wxGetApp().alpStr + " flags added.");
    }
}
