 .386P
 .model FLAT
 externdef _d_zistepu:dword
 externdef _d_pzbuffer:dword
 externdef _d_zistepv:dword
 externdef _d_zrowbytes:dword
 externdef _d_ziorigin:dword
 externdef _r_turb_s:dword
 externdef _r_turb_t:dword
 externdef _r_turb_pdest:dword
 externdef _r_turb_spancount:dword
 externdef _r_turb_turb:dword
 externdef _r_turb_pbase:dword
 externdef _r_turb_sstep:dword
 externdef _r_turb_tstep:dword
 externdef _r_bmodelactive:dword
 externdef _d_sdivzstepu:dword
 externdef _d_tdivzstepu:dword
 externdef _d_sdivzstepv:dword
 externdef _d_tdivzstepv:dword
 externdef _d_sdivzorigin:dword
 externdef _d_tdivzorigin:dword
 externdef _sadjust:dword
 externdef _tadjust:dword
 externdef _bbextents:dword
 externdef _bbextentt:dword
 externdef _cacheblock:dword
 externdef _d_viewbuffer:dword
 externdef _cachewidth:dword
 externdef _d_pzbuffer:dword
 externdef _d_zrowbytes:dword
 externdef _d_zwidth:dword
 externdef _d_scantable:dword
 externdef _r_lightptr:dword
 externdef _r_numvblocks:dword
 externdef _prowdestbase:dword
 externdef _pbasesource:dword
 externdef _r_lightwidth:dword
 externdef _lightright:dword
 externdef _lightrightstep:dword
 externdef _lightdeltastep:dword
 externdef _lightdelta:dword
 externdef _lightright:dword
 externdef _lightdelta:dword
 externdef _sourcetstep:dword
 externdef _surfrowbytes:dword
 externdef _lightrightstep:dword
 externdef _lightdeltastep:dword
 externdef _r_sourcemax:dword
 externdef _r_stepback:dword
 externdef _colormap:dword
 externdef _blocksize:dword
 externdef _sourcesstep:dword
 externdef _lightleft:dword
 externdef _blockdivshift:dword
 externdef _blockdivmask:dword
 externdef _lightleftstep:dword
 externdef _r_origin:dword
 externdef _r_ppn:dword
 externdef _r_pup:dword
 externdef _r_pright:dword
 externdef _ycenter:dword
 externdef _xcenter:dword
 externdef _d_vrectbottom_particle:dword
 externdef _d_vrectright_particle:dword
 externdef _d_vrecty:dword
 externdef _d_vrectx:dword
 externdef _d_pix_shift:dword
 externdef _d_pix_min:dword
 externdef _d_pix_max:dword
 externdef _d_y_aspect_shift:dword
 externdef _screenwidth:dword
 externdef _vright:dword
 externdef _vup:dword
 externdef _vpn:dword
 externdef _BOPS_Error:dword
 externdef _snd_scaletable:dword
 externdef _paintbuffer:dword
 externdef _snd_linear_count:dword
 externdef _snd_p:dword
 externdef _snd_vol:dword
 externdef _snd_out:dword
 externdef _r_leftclipped:dword
 externdef _r_leftenter:dword
 externdef _r_rightclipped:dword
 externdef _r_rightenter:dword
 externdef _modelorg:dword
 externdef _xscale:dword
 externdef _r_refdef:dword
 externdef _yscale:dword
 externdef _r_leftexit:dword
 externdef _r_rightexit:dword
 externdef _r_lastvertvalid:dword
 externdef _cacheoffset:dword
 externdef _newedges:dword
 externdef _removeedges:dword
 externdef _r_pedge:dword
 externdef _r_framecount:dword
 externdef _r_u1:dword
 externdef _r_emitted:dword
 externdef _edge_p:dword
 externdef _surface_p:dword
 externdef _surfaces:dword
 externdef _r_lzi1:dword
 externdef _r_v1:dword
 externdef _r_ceilv1:dword
 externdef _r_nearzi:dword
 externdef _r_nearzionly:dword
 externdef _edge_aftertail:dword
 externdef _edge_tail:dword
 externdef _current_iv:dword
 externdef _edge_head_u_shift20:dword
 externdef _span_p:dword
 externdef _edge_head:dword
 externdef _fv:dword
 externdef _edge_tail_u_shift20:dword
 externdef _r_apverts:dword
 externdef _r_anumverts:dword
 externdef _aliastransform:dword
 externdef _r_avertexnormals:dword
 externdef _r_plightvec:dword
 externdef _r_ambientlight:dword
 externdef _r_shadelight:dword
 externdef _aliasxcenter:dword
 externdef _aliasycenter:dword
 externdef _a_sstepxfrac:dword
 externdef _r_affinetridesc:dword
 externdef _acolormap:dword
 externdef _d_pcolormap:dword
 externdef _r_affinetridesc:dword
 externdef _d_sfrac:dword
 externdef _d_ptex:dword
 externdef _d_pedgespanpackage:dword
 externdef _d_tfrac:dword
 externdef _d_light:dword
 externdef _d_zi:dword
 externdef _d_pdest:dword
 externdef _d_pz:dword
 externdef _d_aspancount:dword
 externdef _erroradjustup:dword
 externdef _errorterm:dword
 externdef _d_xdenom:dword
 externdef _r_p0:dword
 externdef _r_p1:dword
 externdef _r_p2:dword
 externdef _a_tstepxfrac:dword
 externdef _r_sstepx:dword
 externdef _r_tstepx:dword
 externdef _a_ststepxwhole:dword
 externdef _zspantable:dword
 externdef _skintable:dword
 externdef _r_zistepx:dword
 externdef _erroradjustdown:dword
 externdef _d_countextrastep:dword
 externdef _ubasestep:dword
 externdef _a_ststepxwhole:dword
 externdef _a_tstepxfrac:dword
 externdef _r_lstepx:dword
 externdef _a_spans:dword
 externdef _erroradjustdown:dword
 externdef _d_pdestextrastep:dword
 externdef _d_pzextrastep:dword
 externdef _d_sfracextrastep:dword
 externdef _d_ptexextrastep:dword
 externdef _d_countextrastep:dword
 externdef _d_tfracextrastep:dword
 externdef _d_lightextrastep:dword
 externdef _d_ziextrastep:dword
 externdef _d_pdestbasestep:dword
 externdef _d_pzbasestep:dword
 externdef _d_sfracbasestep:dword
 externdef _d_ptexbasestep:dword
 externdef _ubasestep:dword
 externdef _d_tfracbasestep:dword
 externdef _d_lightbasestep:dword
 externdef _d_zibasestep:dword
 externdef _zspantable:dword
 externdef _r_lstepy:dword
 externdef _r_sstepy:dword
 externdef _r_tstepy:dword
 externdef _r_zistepy:dword
 externdef _D_PolysetSetEdgeTable:dword
 externdef _D_RasterizeAliasPolySmooth:dword
 externdef float_point5:dword
 externdef Float2ToThe31nd:dword
 externdef izistep:dword
 externdef izi:dword
 externdef FloatMinus2ToThe31nd:dword
 externdef float_1:dword
 externdef float_particle_z_clip:dword
 externdef float_minus_1:dword
 externdef float_0:dword
 externdef fp_16:dword
 externdef fp_64k:dword
 externdef fp_1m:dword
 externdef fp_1m_minus_1:dword
 externdef fp_8:dword
 externdef entryvec_table:dword
 externdef advancetable:dword
 externdef sstep:dword
 externdef tstep:dword
 externdef pspantemp:dword
 externdef counttemp:dword
 externdef jumptemp:dword
 externdef reciprocal_table:dword
 externdef DP_Count:dword
 externdef DP_u:dword
 externdef DP_v:dword
 externdef DP_32768:dword
 externdef DP_Color:dword
 externdef DP_Pix:dword
 externdef DP_EntryTable:dword
 externdef pbase:dword
 externdef s:dword
 externdef t:dword
 externdef sfracf:dword
 externdef tfracf:dword
 externdef snext:dword
 externdef tnext:dword
 externdef spancountminus1:dword
 externdef zi16stepu:dword
 externdef sdivz16stepu:dword
 externdef tdivz16stepu:dword
 externdef zi8stepu:dword
 externdef sdivz8stepu:dword
 externdef tdivz8stepu:dword
 externdef reciprocal_table_16:dword
 externdef entryvec_table_16:dword
 externdef ceil_cw:dword
 externdef single_cw:dword
 externdef fp_64kx64k:dword
 externdef pz:dword
 externdef spr8entryvec_table:dword
_DATA SEGMENT
Ltemp dd 0
float_1_div_0100000h dd 035800000h
float_point_999 dd 0.999
float_1_point_001 dd 1.001
_DATA ENDS
_TEXT SEGMENT
 public _R_EdgeCodeStart
_R_EdgeCodeStart:
 public _R_InsertNewEdges
_R_InsertNewEdges:
 push edi
 push esi
 mov edx,ds:dword ptr[4+8+esp]
 push ebx
 mov ecx,ds:dword ptr[8+12+esp]
LDoNextEdge:
 mov eax,ds:dword ptr[0+edx]
 mov edi,edx
LContinueSearch:
 mov ebx,ds:dword ptr[0+ecx]
 mov esi,ds:dword ptr[12+ecx]
 cmp eax,ebx
 jle LAddedge
 mov ebx,ds:dword ptr[0+esi]
 mov ecx,ds:dword ptr[12+esi]
 cmp eax,ebx
 jle LAddedge2
 mov ebx,ds:dword ptr[0+ecx]
 mov esi,ds:dword ptr[12+ecx]
 cmp eax,ebx
 jle LAddedge
 mov ebx,ds:dword ptr[0+esi]
 mov ecx,ds:dword ptr[12+esi]
 cmp eax,ebx
 jg LContinueSearch
LAddedge2:
 mov edx,ds:dword ptr[12+edx]
 mov ebx,ds:dword ptr[8+esi]
 mov ds:dword ptr[12+edi],esi
 mov ds:dword ptr[8+edi],ebx
 mov ds:dword ptr[12+ebx],edi
 mov ds:dword ptr[8+esi],edi
 mov ecx,esi
 cmp edx,0
 jnz LDoNextEdge
 jmp LDone
 align 4
LAddedge:
 mov edx,ds:dword ptr[12+edx]
 mov ebx,ds:dword ptr[8+ecx]
 mov ds:dword ptr[12+edi],ecx
 mov ds:dword ptr[8+edi],ebx
 mov ds:dword ptr[12+ebx],edi
 mov ds:dword ptr[8+ecx],edi
 cmp edx,0
 jnz LDoNextEdge
LDone:
 pop ebx
 pop esi
 pop edi
 ret
 public _R_RemoveEdges
_R_RemoveEdges:
 push ebx
 mov eax,ds:dword ptr[4+4+esp]
Lre_loop:
 mov ecx,ds:dword ptr[12+eax]
 mov ebx,ds:dword ptr[20+eax]
 mov edx,ds:dword ptr[8+eax]
 test ebx,ebx
 mov ds:dword ptr[8+ecx],edx
 jz Lre_done
 mov ds:dword ptr[12+edx],ecx
 mov ecx,ds:dword ptr[12+ebx]
 mov edx,ds:dword ptr[8+ebx]
 mov eax,ds:dword ptr[20+ebx]
 mov ds:dword ptr[8+ecx],edx
 test eax,eax
 mov ds:dword ptr[12+edx],ecx
 jnz Lre_loop
 pop ebx
 ret
Lre_done:
 mov ds:dword ptr[12+edx],ecx
 pop ebx
 ret
 public _R_StepActiveU
_R_StepActiveU:
 push edi
 mov edx,ds:dword ptr[4+4+esp]
 push esi
 push ebx
 mov esi,ds:dword ptr[8+edx]
LNewEdge:
 mov edi,ds:dword ptr[0+esi]
LNextEdge:
 mov eax,ds:dword ptr[0+edx]
 mov ebx,ds:dword ptr[4+edx]
 add eax,ebx
 mov esi,ds:dword ptr[12+edx]
 mov ds:dword ptr[0+edx],eax
 cmp eax,edi
 jl LPushBack
 mov edi,ds:dword ptr[0+esi]
 mov ebx,ds:dword ptr[4+esi]
 add edi,ebx
 mov edx,ds:dword ptr[12+esi]
 mov ds:dword ptr[0+esi],edi
 cmp edi,eax
 jl LPushBack2
 mov eax,ds:dword ptr[0+edx]
 mov ebx,ds:dword ptr[4+edx]
 add eax,ebx
 mov esi,ds:dword ptr[12+edx]
 mov ds:dword ptr[0+edx],eax
 cmp eax,edi
 jl LPushBack
 mov edi,ds:dword ptr[0+esi]
 mov ebx,ds:dword ptr[4+esi]
 add edi,ebx
 mov edx,ds:dword ptr[12+esi]
 mov ds:dword ptr[0+esi],edi
 cmp edi,eax
 jnl LNextEdge
LPushBack2:
 mov ebx,edx
 mov eax,edi
 mov edx,esi
 mov esi,ebx
LPushBack:
 mov ecx,ds:dword ptr[8+edx]
 mov ebx,ds:dword ptr[12+edx]
 cmp edx,offset _edge_aftertail
 jz LUDone
 mov edi,ds:dword ptr[8+ecx]
 mov ds:dword ptr[8+esi],ecx
 mov ds:dword ptr[12+ecx],ebx
LPushBackLoop:
 mov ecx,ds:dword ptr[8+edi]
 mov ebx,ds:dword ptr[0+edi]
 cmp eax,ebx
 jnl LPushBackFound
 mov edi,ds:dword ptr[8+ecx]
 mov ebx,ds:dword ptr[0+ecx]
 cmp eax,ebx
 jl LPushBackLoop
 mov edi,ecx
LPushBackFound:
 mov ebx,ds:dword ptr[12+edi]
 mov ds:dword ptr[8+edx],edi
 mov ds:dword ptr[12+edx],ebx
 mov ds:dword ptr[12+edi],edx
 mov ds:dword ptr[8+ebx],edx
 mov edx,esi
 mov esi,ds:dword ptr[8+esi]
 cmp edx,offset _edge_tail
 jnz LNewEdge
LUDone:
 pop ebx
 pop esi
 pop edi
 ret
 align 4
TrailingEdge:
 mov eax,ds:dword ptr[20+esi]
 dec eax
 jnz LInverted
 mov ds:dword ptr[20+esi],eax
 mov ecx,ds:dword ptr[40+esi]
 mov edx,ds:dword ptr[12345678h]
LPatch0:
 mov eax,ds:dword ptr[_r_bmodelactive]
 sub eax,ecx
 cmp edx,esi
 mov ds:dword ptr[_r_bmodelactive],eax
 jnz LNoEmit
 mov eax,ds:dword ptr[0+ebx]
 shr eax,20
 mov edx,ds:dword ptr[16+esi]
 mov ecx,ds:dword ptr[0+esi]
 cmp eax,edx
 jle LNoEmit2
 mov ds:dword ptr[16+ecx],eax
 sub eax,edx
 mov ds:dword ptr[0+ebp],edx
 mov ds:dword ptr[8+ebp],eax
 mov eax,ds:dword ptr[_current_iv]
 mov ds:dword ptr[4+ebp],eax
 mov eax,ds:dword ptr[8+esi]
 mov ds:dword ptr[12+ebp],eax
 mov ds:dword ptr[8+esi],ebp
 add ebp,16
 mov edx,ds:dword ptr[0+esi]
 mov esi,ds:dword ptr[4+esi]
 mov ds:dword ptr[0+esi],edx
 mov ds:dword ptr[4+edx],esi
 ret
LNoEmit2:
 mov ds:dword ptr[16+ecx],eax
 mov edx,ds:dword ptr[0+esi]
 mov esi,ds:dword ptr[4+esi]
 mov ds:dword ptr[0+esi],edx
 mov ds:dword ptr[4+edx],esi
 ret
LNoEmit:
 mov edx,ds:dword ptr[0+esi]
 mov esi,ds:dword ptr[4+esi]
 mov ds:dword ptr[0+esi],edx
 mov ds:dword ptr[4+edx],esi
 ret
LInverted:
 mov ds:dword ptr[20+esi],eax
 ret
Lgs_trailing:
 push offset Lgs_nextedge
 jmp TrailingEdge
 public _R_GenerateSpans
_R_GenerateSpans:
 push ebp
 push edi
 push esi
 push ebx
 mov eax,ds:dword ptr[_surfaces]
 mov edx,ds:dword ptr[_edge_head_u_shift20]
 add eax,64
 mov ebp,ds:dword ptr[_span_p]
 mov ds:dword ptr[_r_bmodelactive],0
 mov ds:dword ptr[0+eax],eax
 mov ds:dword ptr[4+eax],eax
 mov ds:dword ptr[16+eax],edx
 mov ebx,ds:dword ptr[_edge_head+12]
 cmp ebx,offset _edge_tail
 jz Lgs_lastspan
Lgs_edgeloop:
 mov edi,ds:dword ptr[16+ebx]
 mov eax,ds:dword ptr[_surfaces]
 mov esi,edi
 and edi,0FFFF0000h
 and esi,0FFFFh
 jz Lgs_leading
 shl esi,6
 add esi,eax
 test edi,edi
 jz Lgs_trailing
 call near ptr TrailingEdge
 mov eax,ds:dword ptr[_surfaces]
Lgs_leading:
 shr edi,16-6
 mov eax,ds:dword ptr[_surfaces]
 add edi,eax
 mov esi,ds:dword ptr[12345678h]
LPatch2:
 mov edx,ds:dword ptr[20+edi]
 mov eax,ds:dword ptr[40+edi]
 test eax,eax
 jnz Lbmodel_leading
 test edx,edx
 jnz Lxl_done
 inc edx
 mov eax,ds:dword ptr[12+edi]
 mov ds:dword ptr[20+edi],edx
 mov ecx,ds:dword ptr[12+esi]
 cmp eax,ecx
 jl Lnewtop
Lsortloopnb:
 mov esi,ds:dword ptr[0+esi]
 mov ecx,ds:dword ptr[12+esi]
 cmp eax,ecx
 jge Lsortloopnb
 jmp LInsertAndExit
 align 4
Lbmodel_leading:
 test edx,edx
 jnz Lxl_done
 mov ecx,ds:dword ptr[_r_bmodelactive]
 inc edx
 inc ecx
 mov ds:dword ptr[20+edi],edx
 mov ds:dword ptr[_r_bmodelactive],ecx
 mov eax,ds:dword ptr[12+edi]
 mov ecx,ds:dword ptr[12+esi]
 cmp eax,ecx
 jl Lnewtop
 jz Lzcheck_for_newtop
Lsortloop:
 mov esi,ds:dword ptr[0+esi]
 mov ecx,ds:dword ptr[12+esi]
 cmp eax,ecx
 jg Lsortloop
 jne LInsertAndExit
 mov eax,ds:dword ptr[0+ebx]
 sub eax,0FFFFFh
 mov ds:dword ptr[Ltemp],eax
 fild ds:dword ptr[Ltemp]
 fmul ds:dword ptr[float_1_div_0100000h]
 fld st(0)
 fmul ds:dword ptr[48+edi]
 fld ds:dword ptr[_fv]
 fmul ds:dword ptr[52+edi]
 fxch st(1)
 fadd ds:dword ptr[44+edi]
 fld ds:dword ptr[48+esi]
 fmul st(0),st(3)
 fxch st(1)
 faddp st(2),st(0)
 fld ds:dword ptr[_fv]
 fmul ds:dword ptr[52+esi]
 fld st(2)
 fmul ds:dword ptr[float_point_999]
 fxch st(2)
 fadd ds:dword ptr[44+esi]
 faddp st(1),st(0)
 fxch st(1)
 fcomp st(1)
 fxch st(1)
 fmul ds:dword ptr[float_1_point_001]
 fxch st(1)
 fnstsw ax
 test ah,001h
 jz Lgotposition_fpop3
 fcomp st(1)
 fnstsw ax
 test ah,045h
 jz Lsortloop_fpop2
 fld ds:dword ptr[48+edi]
 fcomp ds:dword ptr[48+esi]
 fnstsw ax
 test ah,001h
 jz Lgotposition_fpop2
 fstp st(0)
 fstp st(0)
 mov eax,ds:dword ptr[12+edi]
 jmp Lsortloop
Lgotposition_fpop3:
 fstp st(0)
Lgotposition_fpop2:
 fstp st(0)
 fstp st(0)
 jmp LInsertAndExit
Lnewtop_fpop3:
 fstp st(0)
Lnewtop_fpop2:
 fstp st(0)
 fstp st(0)
 mov eax,ds:dword ptr[12+edi]
Lnewtop:
 mov eax,ds:dword ptr[0+ebx]
 mov edx,ds:dword ptr[16+esi]
 shr eax,20
 mov ds:dword ptr[16+edi],eax
 cmp eax,edx
 jle LInsertAndExit
 sub eax,edx
 mov ds:dword ptr[0+ebp],edx
 mov ds:dword ptr[8+ebp],eax
 mov eax,ds:dword ptr[_current_iv]
 mov ds:dword ptr[4+ebp],eax
 mov eax,ds:dword ptr[8+esi]
 mov ds:dword ptr[12+ebp],eax
 mov ds:dword ptr[8+esi],ebp
 add ebp,16
LInsertAndExit:
 mov ds:dword ptr[0+edi],esi
 mov eax,ds:dword ptr[4+esi]
 mov ds:dword ptr[4+edi],eax
 mov ds:dword ptr[4+esi],edi
 mov ds:dword ptr[0+eax],edi
Lgs_nextedge:
 mov ebx,ds:dword ptr[12+ebx]
 cmp ebx,offset _edge_tail
 jnz Lgs_edgeloop
Lgs_lastspan:
 mov esi,ds:dword ptr[12345678h]
LPatch3:
 mov eax,ds:dword ptr[_edge_tail_u_shift20]
 xor ecx,ecx
 mov edx,ds:dword ptr[16+esi]
 sub eax,edx
 jle Lgs_resetspanstate
 mov ds:dword ptr[0+ebp],edx
 mov ds:dword ptr[8+ebp],eax
 mov eax,ds:dword ptr[_current_iv]
 mov ds:dword ptr[4+ebp],eax
 mov eax,ds:dword ptr[8+esi]
 mov ds:dword ptr[12+ebp],eax
 mov ds:dword ptr[8+esi],ebp
 add ebp,16
Lgs_resetspanstate:
 mov ds:dword ptr[20+esi],ecx
 mov esi,ds:dword ptr[0+esi]
 cmp esi,012345678h
LPatch4:
 jnz Lgs_resetspanstate
 mov ds:dword ptr[_span_p],ebp
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
Lxl_done:
 inc edx
 mov ds:dword ptr[20+edi],edx
 jmp Lgs_nextedge
 align 4
Lzcheck_for_newtop:
 mov eax,ds:dword ptr[0+ebx]
 sub eax,0FFFFFh
 mov ds:dword ptr[Ltemp],eax
 fild ds:dword ptr[Ltemp]
 fmul ds:dword ptr[float_1_div_0100000h]
 fld st(0)
 fmul ds:dword ptr[48+edi]
 fld ds:dword ptr[_fv]
 fmul ds:dword ptr[52+edi]
 fxch st(1)
 fadd ds:dword ptr[44+edi]
 fld ds:dword ptr[48+esi]
 fmul st(0),st(3)
 fxch st(1)
 faddp st(2),st(0)
 fld ds:dword ptr[_fv]
 fmul ds:dword ptr[52+esi]
 fld st(2)
 fmul ds:dword ptr[float_point_999]
 fxch st(2)
 fadd ds:dword ptr[44+esi]
 faddp st(1),st(0)
 fxch st(1)
 fcomp st(1)
 fxch st(1)
 fmul ds:dword ptr[float_1_point_001]
 fxch st(1)
 fnstsw ax
 test ah,001h
 jz Lnewtop_fpop3
 fcomp st(1)
 fnstsw ax
 test ah,045h
 jz Lsortloop_fpop2
 fld ds:dword ptr[48+edi]
 fcomp ds:dword ptr[48+esi]
 fnstsw ax
 test ah,001h
 jz Lnewtop_fpop2
Lsortloop_fpop2:
 fstp st(0)
 fstp st(0)
 mov eax,ds:dword ptr[12+edi]
 jmp Lsortloop
 public _R_EdgeCodeEnd
_R_EdgeCodeEnd:
 align 4
 public _R_SurfacePatch
_R_SurfacePatch:
 mov eax,ds:dword ptr[_surfaces]
 add eax,64
 mov ds:dword ptr[LPatch4-4],eax
 add eax,0
 mov ds:dword ptr[LPatch0-4],eax
 mov ds:dword ptr[LPatch2-4],eax
 mov ds:dword ptr[LPatch3-4],eax
 ret
_TEXT ENDS
 END
