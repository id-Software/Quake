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
 align 4
 public _d_sdivzstepu
 public _d_tdivzstepu
 public _d_zistepu
 public _d_sdivzstepv
 public _d_tdivzstepv
 public _d_zistepv
 public _d_sdivzorigin
 public _d_tdivzorigin
 public _d_ziorigin
_d_sdivzstepu dd 0
_d_tdivzstepu dd 0
_d_zistepu dd 0
_d_sdivzstepv dd 0
_d_tdivzstepv dd 0
_d_zistepv dd 0
_d_sdivzorigin dd 0
_d_tdivzorigin dd 0
_d_ziorigin dd 0
 public _sadjust
 public _tadjust
 public _bbextents
 public _bbextentt
_sadjust dd 0
_tadjust dd 0
_bbextents dd 0
_bbextentt dd 0
 public _cacheblock
 public _d_viewbuffer
 public _cachewidth
 public _d_pzbuffer
 public _d_zrowbytes
 public _d_zwidth
_cacheblock dd 0
_cachewidth dd 0
_d_viewbuffer dd 0
_d_pzbuffer dd 0
_d_zrowbytes dd 0
_d_zwidth dd 0
 public izi
izi dd 0
 public pbase, s, t, sfracf, tfracf, snext, tnext
 public spancountminus1, zi16stepu, sdivz16stepu, tdivz16stepu
 public zi8stepu, sdivz8stepu, tdivz8stepu, pz
s dd 0
t dd 0
snext dd 0
tnext dd 0
sfracf dd 0
tfracf dd 0
pbase dd 0
zi8stepu dd 0
sdivz8stepu dd 0
tdivz8stepu dd 0
zi16stepu dd 0
sdivz16stepu dd 0
tdivz16stepu dd 0
spancountminus1 dd 0
pz dd 0
 public izistep
izistep dd 0
 public reciprocal_table_16, entryvec_table_16
reciprocal_table_16 dd 040000000h, 02aaaaaaah, 020000000h
 dd 019999999h, 015555555h, 012492492h
 dd 010000000h, 0e38e38eh, 0ccccccch, 0ba2e8bah
 dd 0aaaaaaah, 09d89d89h, 09249249h, 08888888h
 externdef Entry2_16:dword
 externdef Entry3_16:dword
 externdef Entry4_16:dword
 externdef Entry5_16:dword
 externdef Entry6_16:dword
 externdef Entry7_16:dword
 externdef Entry8_16:dword
 externdef Entry9_16:dword
 externdef Entry10_16:dword
 externdef Entry11_16:dword
 externdef Entry12_16:dword
 externdef Entry13_16:dword
 externdef Entry14_16:dword
 externdef Entry15_16:dword
 externdef Entry16_16:dword
entryvec_table_16 dd 0, Entry2_16, Entry3_16, Entry4_16
 dd Entry5_16, Entry6_16, Entry7_16, Entry8_16
 dd Entry9_16, Entry10_16, Entry11_16, Entry12_16
 dd Entry13_16, Entry14_16, Entry15_16, Entry16_16
 public DP_Count, DP_u, DP_v, DP_32768, DP_Color, DP_Pix, DP_EntryTable
DP_Count dd 0
DP_u dd 0
DP_v dd 0
DP_32768 dd 32768.0
DP_Color dd 0
DP_Pix dd 0
 externdef DP_1x1:dword
 externdef DP_2x2:dword
 externdef DP_3x3:dword
 externdef DP_4x4:dword
DP_EntryTable dd DP_1x1, DP_2x2, DP_3x3, DP_4x4
 public advancetable, sstep, tstep, pspantemp, counttemp, jumptemp
advancetable dd 0, 0
sstep dd 0
tstep dd 0
pspantemp dd 0
counttemp dd 0
jumptemp dd 0
 public reciprocal_table, entryvec_table
reciprocal_table dd 040000000h, 02aaaaaaah, 020000000h
 dd 019999999h, 015555555h, 012492492h
 externdef Entry2_8:dword
 externdef Entry3_8:dword
 externdef Entry4_8:dword
 externdef Entry5_8:dword
 externdef Entry6_8:dword
 externdef Entry7_8:dword
 externdef Entry8_8:dword
entryvec_table dd 0, Entry2_8, Entry3_8, Entry4_8
 dd Entry5_8, Entry6_8, Entry7_8, Entry8_8
 externdef Spr8Entry2_8:dword
 externdef Spr8Entry3_8:dword
 externdef Spr8Entry4_8:dword
 externdef Spr8Entry5_8:dword
 externdef Spr8Entry6_8:dword
 externdef Spr8Entry7_8:dword
 externdef Spr8Entry8_8:dword
 public spr8entryvec_table
spr8entryvec_table dd 0, Spr8Entry2_8, Spr8Entry3_8, Spr8Entry4_8
 dd Spr8Entry5_8, Spr8Entry6_8, Spr8Entry7_8, Spr8Entry8_8
_DATA ENDS
 END
