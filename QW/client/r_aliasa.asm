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
Lfloat_1 dd 1.0
Ltemp dd 0
Lcoords dd 0, 0, 0
_DATA ENDS
_TEXT SEGMENT
 public _R_AliasTransformAndProjectFinalVerts
_R_AliasTransformAndProjectFinalVerts:
 push ebp
 push edi
 push esi
 mov esi,ds:dword ptr[_r_apverts]
 mov ebp,ds:dword ptr[12+8+esp]
 mov edi,ds:dword ptr[12+4+esp]
 mov ecx,ds:dword ptr[_r_anumverts]
 sub edx,edx
Lloop:
 mov dl,ds:byte ptr[esi]
 mov ds:byte ptr[Lcoords],dl
 fild ds:dword ptr[Lcoords]
 mov dl,ds:byte ptr[1+esi]
 mov ds:byte ptr[Lcoords+4],dl
 fild ds:dword ptr[Lcoords+4]
 mov dl,ds:byte ptr[2+esi]
 mov ds:byte ptr[Lcoords+8],dl
 fild ds:dword ptr[Lcoords+8]
 fld st(2)
 fmul ds:dword ptr[_aliastransform+32]
 fld st(2)
 fmul ds:dword ptr[_aliastransform+36]
 fxch st(1)
 fadd ds:dword ptr[_aliastransform+44]
 fld st(2)
 fmul ds:dword ptr[_aliastransform+40]
 fxch st(1)
 faddp st(2),st(0)
 mov dl,ds:byte ptr[3+esi]
 mov eax,ds:dword ptr[4+ebp]
 mov ds:dword ptr[0+8+edi],eax
 faddp st(1),st(0)
 mov eax,ds:dword ptr[8+ebp]
 mov ds:dword ptr[0+12+edi],eax
 fdivr ds:dword ptr[Lfloat_1]
 mov eax,ds:dword ptr[0+ebp]
 mov ds:dword ptr[24+edi],eax
 mov eax,ds:dword ptr[32+edi]
 mov eax,ds:dword ptr[12+ebp]
 mov eax,ds:dword ptr[4+esi]
 lea eax,ds:dword ptr[edx+edx*2]
 fxch st(3)
 fld ds:dword ptr[_r_avertexnormals+eax*4]
 fmul ds:dword ptr[_r_plightvec]
 fld ds:dword ptr[_r_avertexnormals+4+eax*4]
 fmul ds:dword ptr[_r_plightvec+4]
 fld ds:dword ptr[_r_avertexnormals+8+eax*4]
 fmul ds:dword ptr[_r_plightvec+8]
 fxch st(1)
 faddp st(2),st(0)
 fld st(2)
 fmul ds:dword ptr[_aliastransform+0]
 fxch st(2)
 faddp st(1),st(0)
 fst ds:dword ptr[Ltemp]
 mov eax,ds:dword ptr[_r_ambientlight]
 mov dl,ds:byte ptr[Ltemp+3]
 test dl,080h
 jz Lsavelight
 fmul ds:dword ptr[_r_shadelight]
 fistp ds:dword ptr[Ltemp]
 add eax,ds:dword ptr[Ltemp]
 jns Lp1
 sub eax,eax
Lp1:
 fxch st(1)
 fmul ds:dword ptr[_aliastransform+16]
 fxch st(3)
 fld st(0)
 fmul ds:dword ptr[_aliastransform+4]
 fxch st(1)
 mov ds:dword ptr[0+16+edi],eax
 fmul ds:dword ptr[_aliastransform+20]
 fxch st(2)
 fadd ds:dword ptr[_aliastransform+12]
 fxch st(4)
 fadd ds:dword ptr[_aliastransform+28]
 fxch st(3)
 fld st(0)
 fmul ds:dword ptr[_aliastransform+8]
 fxch st(1)
 fmul ds:dword ptr[_aliastransform+24]
 fxch st(5)
 faddp st(2),st(0)
 fxch st(3)
 faddp st(2),st(0)
 add esi,4
 faddp st(2),st(0)
 faddp st(2),st(0)
 add ebp,12
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fadd ds:dword ptr[_aliasxcenter]
 fxch st(1)
 fadd ds:dword ptr[_aliasycenter]
 fxch st(2)
 fistp ds:dword ptr[0+20+edi]
 fistp ds:dword ptr[0+0+edi]
 fistp ds:dword ptr[0+4+edi]
 add edi,32
 dec ecx
 jnz Lloop
 pop esi
 pop edi
 pop ebp
 ret
Lsavelight:
 fstp st(0)
 jmp Lp1
_TEXT ENDS
 END
