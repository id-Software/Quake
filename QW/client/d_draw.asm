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
_TEXT SEGMENT
LClampHigh0:
 mov esi,ds:dword ptr[_bbextents]
 jmp LClampReentry0
LClampHighOrLow0:
 jg LClampHigh0
 xor esi,esi
 jmp LClampReentry0
LClampHigh1:
 mov edx,ds:dword ptr[_bbextentt]
 jmp LClampReentry1
LClampHighOrLow1:
 jg LClampHigh1
 xor edx,edx
 jmp LClampReentry1
LClampLow2:
 mov ebp,2048
 jmp LClampReentry2
LClampHigh2:
 mov ebp,ds:dword ptr[_bbextents]
 jmp LClampReentry2
LClampLow3:
 mov ecx,2048
 jmp LClampReentry3
LClampHigh3:
 mov ecx,ds:dword ptr[_bbextentt]
 jmp LClampReentry3
LClampLow4:
 mov eax,2048
 jmp LClampReentry4
LClampHigh4:
 mov eax,ds:dword ptr[_bbextents]
 jmp LClampReentry4
LClampLow5:
 mov ebx,2048
 jmp LClampReentry5
LClampHigh5:
 mov ebx,ds:dword ptr[_bbextentt]
 jmp LClampReentry5
 align 4
 public _D_DrawSpans8
_D_DrawSpans8:
 push ebp
 push edi
 push esi
 push ebx
 fld ds:dword ptr[_d_sdivzstepu]
 fmul ds:dword ptr[fp_8]
 mov edx,ds:dword ptr[_cacheblock]
 fld ds:dword ptr[_d_tdivzstepu]
 fmul ds:dword ptr[fp_8]
 mov ebx,ds:dword ptr[4+16+esp]
 fld ds:dword ptr[_d_zistepu]
 fmul ds:dword ptr[fp_8]
 mov ds:dword ptr[pbase],edx
 fstp ds:dword ptr[zi8stepu]
 fstp ds:dword ptr[tdivz8stepu]
 fstp ds:dword ptr[sdivz8stepu]
LSpanLoop:
 fild ds:dword ptr[4+ebx]
 fild ds:dword ptr[0+ebx]
 fld st(1)
 fmul ds:dword ptr[_d_sdivzstepv]
 fld st(1)
 fmul ds:dword ptr[_d_sdivzstepu]
 fld st(2)
 fmul ds:dword ptr[_d_tdivzstepu]
 fxch st(1)
 faddp st(2),st(0)
 fxch st(1)
 fld st(3)
 fmul ds:dword ptr[_d_tdivzstepv]
 fxch st(1)
 fadd ds:dword ptr[_d_sdivzorigin]
 fxch st(4)
 fmul ds:dword ptr[_d_zistepv]
 fxch st(1)
 faddp st(2),st(0)
 fxch st(2)
 fmul ds:dword ptr[_d_zistepu]
 fxch st(1)
 fadd ds:dword ptr[_d_tdivzorigin]
 fxch st(2)
 faddp st(1),st(0)
 fld ds:dword ptr[fp_64k]
 fxch st(1)
 fadd ds:dword ptr[_d_ziorigin]
 fdiv st(1),st(0)
 mov ecx,ds:dword ptr[_d_viewbuffer]
 mov eax,ds:dword ptr[4+ebx]
 mov ds:dword ptr[pspantemp],ebx
 mov edx,ds:dword ptr[_tadjust]
 mov esi,ds:dword ptr[_sadjust]
 mov edi,ds:dword ptr[_d_scantable+eax*4]
 add edi,ecx
 mov ecx,ds:dword ptr[0+ebx]
 add edi,ecx
 mov ecx,ds:dword ptr[8+ebx]
 cmp ecx,8
 ja LSetupNotLast1
 dec ecx
 jz LCleanup1
 mov ds:dword ptr[spancountminus1],ecx
 fxch st(1)
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[s]
 fistp ds:dword ptr[t]
 fild ds:dword ptr[spancountminus1]
 fld ds:dword ptr[_d_tdivzstepu]
 fld ds:dword ptr[_d_zistepu]
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(2)
 fmul ds:dword ptr[_d_sdivzstepu]
 fxch st(1)
 faddp st(3),st(0)
 fxch st(1)
 faddp st(3),st(0)
 faddp st(3),st(0)
 fld ds:dword ptr[fp_64k]
 fdiv st(0),st(1)
 jmp LFDIVInFlight1
LCleanup1:
 fxch st(1)
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[s]
 fistp ds:dword ptr[t]
 jmp LFDIVInFlight1
 align 4
LSetupNotLast1:
 fxch st(1)
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[s]
 fistp ds:dword ptr[t]
 fadd ds:dword ptr[zi8stepu]
 fxch st(2)
 fadd ds:dword ptr[sdivz8stepu]
 fxch st(2)
 fld ds:dword ptr[tdivz8stepu]
 faddp st(2),st(0)
 fld ds:dword ptr[fp_64k]
 fdiv st(0),st(1)
LFDIVInFlight1:
 add esi,ds:dword ptr[s]
 add edx,ds:dword ptr[t]
 mov ebx,ds:dword ptr[_bbextents]
 mov ebp,ds:dword ptr[_bbextentt]
 cmp esi,ebx
 ja LClampHighOrLow0
LClampReentry0:
 mov ds:dword ptr[s],esi
 mov ebx,ds:dword ptr[pbase]
 shl esi,16
 cmp edx,ebp
 mov ds:dword ptr[sfracf],esi
 ja LClampHighOrLow1
LClampReentry1:
 mov ds:dword ptr[t],edx
 mov esi,ds:dword ptr[s]
 shl edx,16
 mov eax,ds:dword ptr[t]
 sar esi,16
 mov ds:dword ptr[tfracf],edx
 sar eax,16
 mov edx,ds:dword ptr[_cachewidth]
 imul eax,edx
 add esi,ebx
 add esi,eax
 cmp ecx,8
 jna LLastSegment
LNotLastSegment:
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[snext]
 fistp ds:dword ptr[tnext]
 mov eax,ds:dword ptr[snext]
 mov edx,ds:dword ptr[tnext]
 mov bl,ds:byte ptr[esi]
 sub ecx,8
 mov ebp,ds:dword ptr[_sadjust]
 mov ds:dword ptr[counttemp],ecx
 mov ecx,ds:dword ptr[_tadjust]
 mov ds:byte ptr[edi],bl
 add ebp,eax
 add ecx,edx
 mov eax,ds:dword ptr[_bbextents]
 mov edx,ds:dword ptr[_bbextentt]
 cmp ebp,2048
 jl LClampLow2
 cmp ebp,eax
 ja LClampHigh2
LClampReentry2:
 cmp ecx,2048
 jl LClampLow3
 cmp ecx,edx
 ja LClampHigh3
LClampReentry3:
 mov ds:dword ptr[snext],ebp
 mov ds:dword ptr[tnext],ecx
 sub ebp,ds:dword ptr[s]
 sub ecx,ds:dword ptr[t]
 mov eax,ecx
 mov edx,ebp
 sar eax,19
 jz LZero
 sar edx,19
 mov ebx,ds:dword ptr[_cachewidth]
 imul eax,ebx
 jmp LSetUp1
LZero:
 sar edx,19
 mov ebx,ds:dword ptr[_cachewidth]
LSetUp1:
 add eax,edx
 mov edx,ds:dword ptr[tfracf]
 mov ds:dword ptr[advancetable+4],eax
 add eax,ebx
 shl ebp,13
 mov ebx,ds:dword ptr[sfracf]
 shl ecx,13
 mov ds:dword ptr[advancetable],eax
 mov ds:dword ptr[tstep],ecx
 add edx,ecx
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov al,ds:byte ptr[esi]
 add ebx,ebp
 mov ds:byte ptr[1+edi],al
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[2+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[3+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 mov ecx,ds:dword ptr[counttemp]
 cmp ecx,8
 ja LSetupNotLast2
 dec ecx
 jz LFDIVInFlight2
 mov ds:dword ptr[spancountminus1],ecx
 fild ds:dword ptr[spancountminus1]
 fld ds:dword ptr[_d_zistepu]
 fmul st(0),st(1)
 fld ds:dword ptr[_d_tdivzstepu]
 fmul st(0),st(2)
 fxch st(1)
 faddp st(3),st(0)
 fxch st(1)
 fmul ds:dword ptr[_d_sdivzstepu]
 fxch st(1)
 faddp st(3),st(0)
 fld ds:dword ptr[fp_64k]
 fxch st(1)
 faddp st(4),st(0)
 fdiv st(0),st(1)
 jmp LFDIVInFlight2
 align 4
LSetupNotLast2:
 fadd ds:dword ptr[zi8stepu]
 fxch st(2)
 fadd ds:dword ptr[sdivz8stepu]
 fxch st(2)
 fld ds:dword ptr[tdivz8stepu]
 faddp st(2),st(0)
 fld ds:dword ptr[fp_64k]
 fdiv st(0),st(1)
LFDIVInFlight2:
 mov ds:dword ptr[counttemp],ecx
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[4+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[5+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[6+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edi,8
 mov ds:dword ptr[tfracf],edx
 mov edx,ds:dword ptr[snext]
 mov ds:dword ptr[sfracf],ebx
 mov ebx,ds:dword ptr[tnext]
 mov ds:dword ptr[s],edx
 mov ds:dword ptr[t],ebx
 mov ecx,ds:dword ptr[counttemp]
 cmp ecx,8
 mov ds:byte ptr[-1+edi],al
 ja LNotLastSegment
LLastSegment:
 test ecx,ecx
 jz LNoSteps
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[snext]
 fistp ds:dword ptr[tnext]
 mov al,ds:byte ptr[esi]
 mov ebx,ds:dword ptr[_tadjust]
 mov ds:byte ptr[edi],al
 mov eax,ds:dword ptr[_sadjust]
 add eax,ds:dword ptr[snext]
 add ebx,ds:dword ptr[tnext]
 mov ebp,ds:dword ptr[_bbextents]
 mov edx,ds:dword ptr[_bbextentt]
 cmp eax,2048
 jl LClampLow4
 cmp eax,ebp
 ja LClampHigh4
LClampReentry4:
 mov ds:dword ptr[snext],eax
 cmp ebx,2048
 jl LClampLow5
 cmp ebx,edx
 ja LClampHigh5
LClampReentry5:
 cmp ecx,1
 je LOnlyOneStep
 sub eax,ds:dword ptr[s]
 sub ebx,ds:dword ptr[t]
 add eax,eax
 add ebx,ebx
 imul ds:dword ptr[reciprocal_table-8+ecx*4]
 mov ebp,edx
 mov eax,ebx
 imul ds:dword ptr[reciprocal_table-8+ecx*4]
LSetEntryvec:
 mov ebx,ds:dword ptr[entryvec_table+ecx*4]
 mov eax,edx
 mov ds:dword ptr[jumptemp],ebx
 mov ecx,ebp
 sar edx,16
 mov ebx,ds:dword ptr[_cachewidth]
 sar ecx,16
 imul edx,ebx
 add edx,ecx
 mov ecx,ds:dword ptr[tfracf]
 mov ds:dword ptr[advancetable+4],edx
 add edx,ebx
 shl ebp,16
 mov ebx,ds:dword ptr[sfracf]
 shl eax,16
 mov ds:dword ptr[advancetable],edx
 mov ds:dword ptr[tstep],eax
 mov edx,ecx
 add edx,eax
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 jmp dword ptr[jumptemp]
LNoSteps:
 mov al,ds:byte ptr[esi]
 sub edi,7
 jmp LEndSpan
LOnlyOneStep:
 sub eax,ds:dword ptr[s]
 sub ebx,ds:dword ptr[t]
 mov ebp,eax
 mov edx,ebx
 jmp LSetEntryvec
 public Entry2_8
Entry2_8:
 sub edi,6
 mov al,ds:byte ptr[esi]
 jmp LLEntry2_8
 public Entry3_8
Entry3_8:
 sub edi,5
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 jmp LLEntry3_8
 public Entry4_8
Entry4_8:
 sub edi,4
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LLEntry4_8
 public Entry5_8
Entry5_8:
 sub edi,3
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LLEntry5_8
 public Entry6_8
Entry6_8:
 sub edi,2
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LLEntry6_8
 public Entry7_8
Entry7_8:
 dec edi
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LLEntry7_8
 public Entry8_8
Entry8_8:
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[1+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LLEntry7_8:
 sbb ecx,ecx
 mov ds:byte ptr[2+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LLEntry6_8:
 sbb ecx,ecx
 mov ds:byte ptr[3+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LLEntry5_8:
 sbb ecx,ecx
 mov ds:byte ptr[4+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LLEntry4_8:
 sbb ecx,ecx
 mov ds:byte ptr[5+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
LLEntry3_8:
 mov ds:byte ptr[6+edi],al
 mov al,ds:byte ptr[esi]
LLEntry2_8:
LEndSpan:
 fstp st(0)
 fstp st(0)
 fstp st(0)
 mov ebx,ds:dword ptr[pspantemp]
 mov ebx,ds:dword ptr[12+ebx]
 test ebx,ebx
 mov ds:byte ptr[7+edi],al
 jnz LSpanLoop
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret

LClamp:
 mov edx,040000000h
 xor ebx,ebx
 fstp st(0)
 jmp LZDraw
LClampNeg:
 mov edx,040000000h
 xor ebx,ebx
 fstp st(0)
 jmp LZDrawNeg
 public _D_DrawZSpans
_D_DrawZSpans:
 push ebp
 push edi
 push esi
 push ebx
 fld ds:dword ptr[_d_zistepu]
 mov eax,ds:dword ptr[_d_zistepu]
 mov esi,ds:dword ptr[4+16+esp]
 test eax,eax
 jz LFNegSpan
 fmul ds:dword ptr[Float2ToThe31nd]
 fistp ds:dword ptr[izistep]
 mov ebx,ds:dword ptr[izistep]
LFSpanLoop:
 fild ds:dword ptr[4+esi]
 fild ds:dword ptr[0+esi]
 mov ecx,ds:dword ptr[4+esi]
 mov edi,ds:dword ptr[_d_pzbuffer]
 fmul ds:dword ptr[_d_zistepu]
 fxch st(1)
 fmul ds:dword ptr[_d_zistepv]
 fxch st(1)
 fadd ds:dword ptr[_d_ziorigin]
 imul ecx,ds:dword ptr[_d_zrowbytes]
 faddp st(1),st(0)
 fcom ds:dword ptr[float_point5]
 add edi,ecx
 mov edx,ds:dword ptr[0+esi]
 add edx,edx
 mov ecx,ds:dword ptr[8+esi]
 add edi,edx
 push esi
 fnstsw ax
 test ah,045h
 jz LClamp
 fmul ds:dword ptr[Float2ToThe31nd]
 fistp ds:dword ptr[izi]
 mov edx,ds:dword ptr[izi]
LZDraw:
 test edi,2
 jz LFMiddle
 mov eax,edx
 add edx,ebx
 shr eax,16
 dec ecx
 mov ds:word ptr[edi],ax
 add edi,2
LFMiddle:
 push ecx
 shr ecx,1
 jz LFLast
 shr ecx,1
 jnc LFMiddleLoop
 mov eax,edx
 add edx,ebx
 shr eax,16
 mov esi,edx
 add edx,ebx
 and esi,0FFFF0000h
 or eax,esi
 mov ds:dword ptr[edi],eax
 add edi,4
 and ecx,ecx
 jz LFLast
LFMiddleLoop:
 mov eax,edx
 add edx,ebx
 shr eax,16
 mov esi,edx
 add edx,ebx
 and esi,0FFFF0000h
 or eax,esi
 mov ebp,edx
 mov ds:dword ptr[edi],eax
 add edx,ebx
 shr ebp,16
 mov esi,edx
 add edx,ebx
 and esi,0FFFF0000h
 or ebp,esi
 mov ds:dword ptr[4+edi],ebp
 add edi,8
 dec ecx
 jnz LFMiddleLoop
LFLast:
 pop ecx
 pop esi
 and ecx,1
 jz LFSpanDone
 shr edx,16
 mov ds:word ptr[edi],dx
LFSpanDone:
 mov esi,ds:dword ptr[12+esi]
 test esi,esi
 jnz LFSpanLoop
 jmp LFDone
LFNegSpan:
 fmul ds:dword ptr[FloatMinus2ToThe31nd]
 fistp ds:dword ptr[izistep]
 mov ebx,ds:dword ptr[izistep]
LFNegSpanLoop:
 fild ds:dword ptr[4+esi]
 fild ds:dword ptr[0+esi]
 mov ecx,ds:dword ptr[4+esi]
 mov edi,ds:dword ptr[_d_pzbuffer]
 fmul ds:dword ptr[_d_zistepu]
 fxch st(1)
 fmul ds:dword ptr[_d_zistepv]
 fxch st(1)
 fadd ds:dword ptr[_d_ziorigin]
 imul ecx,ds:dword ptr[_d_zrowbytes]
 faddp st(1),st(0)
 fcom ds:dword ptr[float_point5]
 add edi,ecx
 mov edx,ds:dword ptr[0+esi]
 add edx,edx
 mov ecx,ds:dword ptr[8+esi]
 add edi,edx
 push esi
 fnstsw ax
 test ah,045h
 jz LClampNeg
 fmul ds:dword ptr[Float2ToThe31nd]
 fistp ds:dword ptr[izi]
 mov edx,ds:dword ptr[izi]
LZDrawNeg:
 test edi,2
 jz LFNegMiddle
 mov eax,edx
 sub edx,ebx
 shr eax,16
 dec ecx
 mov ds:word ptr[edi],ax
 add edi,2
LFNegMiddle:
 push ecx
 shr ecx,1
 jz LFNegLast
 shr ecx,1
 jnc LFNegMiddleLoop
 mov eax,edx
 sub edx,ebx
 shr eax,16
 mov esi,edx
 sub edx,ebx
 and esi,0FFFF0000h
 or eax,esi
 mov ds:dword ptr[edi],eax
 add edi,4
 and ecx,ecx
 jz LFNegLast
LFNegMiddleLoop:
 mov eax,edx
 sub edx,ebx
 shr eax,16
 mov esi,edx
 sub edx,ebx
 and esi,0FFFF0000h
 or eax,esi
 mov ebp,edx
 mov ds:dword ptr[edi],eax
 sub edx,ebx
 shr ebp,16
 mov esi,edx
 sub edx,ebx
 and esi,0FFFF0000h
 or ebp,esi
 mov ds:dword ptr[4+edi],ebp
 add edi,8
 dec ecx
 jnz LFNegMiddleLoop
LFNegLast:
 pop ecx
 pop esi
 and ecx,1
 jz LFNegSpanDone
 shr edx,16
 mov ds:word ptr[edi],dx
LFNegSpanDone:
 mov esi,ds:dword ptr[12+esi]
 test esi,esi
 jnz LFNegSpanLoop
LFDone:
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
_TEXT ENDS
 END
