' SECCON{LEGACY_VB_P_CODE}

Private Sub Command1_Click() '403F18
  'Data Table: 401B9C
  Dim var_90 As TextBox
  Dim var_F8 As TextBox
  Dim var_88 As TextBox
  Dim var_E4 As Variant
  Dim var_A4 As Variant
  Dim var_C4 As Variant
  Dim var_F4 As Variant
  Dim var_10C As Variant
  Dim var_12C As Variant
  Dim var_14C As Variant
  Dim var_418 As Long
  Dim var_180 As Integer
  Dim var_41C As Long
  Dim var_1D4 As Integer
  Dim var_420 As Long
  Dim var_268 As Integer
  Dim var_424 As Long
  Dim var_2FC As Integer
  Dim var_428 As Long
  Dim var_390 As Integer
  Dim var_42C As Long
  Dim var_B4 As Variant
  Dim var_170 As Variant
  Dim var_11C As Variant
  Dim var_1B4 As String
  Dim var_1C4 As Variant
  Dim var_218 As Integer
  Dim var_238 As String
  Dim var_258 As Variant
  Dim var_2AC As Integer
  Dim var_2CC As String
  Dim var_2EC As Variant
  Dim var_340 As Integer
  Dim var_360 As String
  Dim var_380 As Variant
  Dim var_3D4 As Integer
  Dim var_3F4 As String
  Dim var_414 As Variant
  Dim var_D4 As Variant
  Dim var_8C As String
  Dim var_190 As Variant
  Dim var_13C As Variant
  Dim var_94 As String
  Dim var_FC As String
  Dim var_4B4 As String
  loc_403913: Set var_90 = Me.Text1
  loc_403919: var_94 = var_90.Text
  loc_403925: Set var_F8 = Me.Text1
  loc_40392B: var_FC = var_F8.Text
  loc_403937: Set var_88 = Me.Text1
  loc_40393D: var_8C = var_88.Text
  loc_40394C: var_E4 = (Len(var_8C) < 8)
  loc_403958: var_A4 = CVar(var_94) 'String
  loc_40395E: var_B4 = Left(var_A4, 7)
  loc_403966: var_C4 = "SECCON{"
  loc_403970: var_F4 = var_E4 Or (var_B4 <> var_C4)
  loc_40397C: var_10C = CVar(var_FC) 'String
  loc_403982: var_11C = Right(var_10C, 1)
  loc_40398A: var_12C = "}"
  loc_403994: var_14C = var_F4 Or (var_11C <> var_12C)
  loc_4039B3: If CBool(var_14C) Then
  loc_4039B6:   GoTo loc_badkey
  loc_4039B9: End If
  loc_4039C0: Set var_88 = Me.Text1
  loc_4039C6: var_8C = var_88.Text
  loc_4039D5: var_C4 = "_"
  loc_4039DA: var_A4 = var_C4
  loc_4039E6: var_B4 = Split(var_8C, var_A4, -1, 0)
  loc_4039EE: splitted = var_B4 'Variant
  loc_4039FE: CRefVarAry
  loc_403A0B: If (UBound(splitted, 1) >= 2) Then
  loc_403A0E:   var_C4 = 0
  loc_403A1F:   var_418 = splitted(var_C4)
  loc_403A22:   var_180 = 0
  loc_403A33:   var_41C = splitted(var_180)
  loc_403A36:   var_1D4 = 0
  loc_403A47:   var_420 = splitted(var_1D4)
  loc_403A4A:   var_268 = 0
  loc_403A5B:   var_424 = splitted(var_268)
  loc_403A5E:   var_2FC = 0
  loc_403A6F:   var_428 = splitted(var_2FC)
  loc_403A72:   var_390 = 0
  loc_403A83:   var_42C = splitted(var_390)
  loc_403A86:   var_B4 = 1
  loc_403A96:   var_D4 = Mid(var_418, 8, var_B4)
  loc_403AA1:   var_170 = "L"
  loc_403AAB:   var_11C = 1
  loc_403ABB:   var_13C = Mid(var_41C, 9, var_11C)
  loc_403AC6:   var_1B4 = "E"
  loc_403AD0:   var_1C4 = (var_D4 <> var_170) Or (var_13C <> var_1B4)
  loc_403AD4:   var_218 = 1
  loc_403AE4:   var_228 = Mid(var_420, &HB, var_218)
  loc_403AEF:   var_238 = "A"
  loc_403AF9:   var_258 = var_1C4 Or (var_228 <> var_238)
  loc_403AFD:   var_2AC = 1
  loc_403B0D:   var_2BC = Mid(var_424, &HA, var_2AC)
  loc_403B18:   var_2CC = "G"
  loc_403B22:   var_2EC = var_258 Or (var_2BC <> var_2CC)
  loc_403B26:   var_340 = 1
  loc_403B36:   var_350 = Mid(var_428, &HD, var_340)
  loc_403B41:   var_360 = "Y"
  loc_403B4B:   var_380 = var_2EC Or (var_350 <> var_360)
  loc_403B4F:   var_3D4 = 1
  loc_403B5F:   var_3E4 = Mid(var_42C, &HC, var_3D4)
  loc_403B6A:   var_3F4 = "C"
  loc_403B74:   var_414 = var_380 Or (var_3E4 <> var_3F4)
  loc_403BA1:   If CBool(var_414) Then
  loc_403BA4:     GoTo loc_badkey
  loc_403BA7:   End If
  loc_403BA7:   var_C4 = 0
  loc_403BAC:   var_43C = var_C4 'Variant
  loc_403BB0:   var_180 = 0
  loc_403BB8:   var_C4 = 1
  loc_403BC7:   var_B4 = Len(splitted(var_C4))
  loc_403BCB:   var_12C = 1
  loc_403BD0:   var_D4 = (var_B4 - var_12C)
  loc_403BD7:   For i = 0 To var_D4: i = i 'Variant
  loc_403BEE:     str = splitted(1)
  loc_403C0D:     var_F4 = Mid(str, CLng(i +1), 1)
  loc_403C18:     var_8C = CStr(var_F4)
  loc_403C21:     ch = CVar(Asc(var_8C)) 'Integer
  loc_403C30:     var_11C = (ch * (4 ^ i))
  loc_403C34:     var_13C = (var_43C + var_11C)
  loc_403C38:     var_43C = var_13C 'Variant
  loc_403C4D:   Next i 'Variant
  loc_403C7D:   var_10C = (var_43C <> 350) Or (Len(splitted(1)) <> 2)
  loc_403C86:   If CBool(var_10C) Then
  loc_403C89:     GoTo loc_badkey
  loc_403C8C:   End If
  
  
  loc_403C98:   var_43C = (350 * 256) 'Variant
  loc_403CC3:   For i = 0 To (Len(splitted(2)) - 1): i = i 'Variant
  loc_403CF9:     var_F4 = Mid(splitted(1), CLng((i + 1)), 1)
  loc_403D04:     var_8C = CStr(var_F4)
  loc_403D0D:     ch = CVar(Asc(var_8C)) 'Integer
  loc_403D24:     var_43C = var_43C + (ch * (4 ^ i))
  loc_403D39:   Next i 'Variant
  loc_403D4D:   If CBool(var_43C <> &H15E56) Then
  loc_403D50:     GoTo loc_badkey
  loc_403D53:   End If
  
  
  loc_403D58:   var_43C = 0 'Variant
  loc_403D71:   var_8C = Me.Text1.Text
  loc_403D80:   var_E4 = CVar((Len(var_8C) - 1)) 'Long
  loc_403D8A:   For i = 0 To var_E4: var_44C = i 'Variant
  loc_403D9D:     var_8C = Me.Text1.Text
  loc_403DA5:     var_D4 = 1
  loc_403DAD:     var_C4 = 1
  loc_403DB2:     var_A4 = (var_44C + var_C4)
  loc_403DBB:     var_B4 = CVar(var_8C) 'String
  loc_403DC1:     var_F4 = Mid(var_B4, CLng(var_A4), var_D4)
  loc_403DC9:     var_94 = CStr(var_F4)
  loc_403DD2:     var_170 = CVar(Asc(var_94)) 'Integer
  loc_403DDD:     var_10C = 2 ^ var_44C
  loc_403DE1:     var_11C = (var_170 * var_10C)
  loc_403DE5:     var_13C = (var_43C + var_11C)
  loc_403DE9:     var_43C = var_13C 'Variant
  loc_403E01:   Next i 'Variant
  loc_403E14:   var_8C = Me.Text1.Text
  loc_403E1C:   var_C4 = &H620F3671
  loc_403E36:   var_B4 = CVar(var_8C) 'String
  loc_403E3C:   var_D4 = StrConv(var_B4, &H80, 0)
  loc_403E44:   var_4B0 = var_D4
  loc_403E50:   var_94 = Proc_2_0_403028(var_4B0) 'SHA1, because of some magic constant value from module
  loc_403E5F:   var_E4 = (var_94 = "8B292F1A-9C4631B3-E13CD49C-64EF7454-0352D0C0")
  loc_403E63:   var_F4 = (var_43C = var_C4) And var_E4
  loc_403E78:   If CBool(var_F4) Then
  loc_403E88:     var_94 = Me.Text1.Text
  loc_403E93:     var_C4 = vbNullString
  loc_403E98:     var_B4 = var_C4
  loc_403EAA:     var_8C = "Thank you for your purchase :-) " & vbCrLf
  loc_403EB1:     var_FC = var_8C & "And, "
  loc_403EB8:     var_4B4 = var_FC & var_94
  loc_403EBF:     var_A4 = CVar(var_4B4 & " is Flag.") 'String
  loc_403EC2:     MsgBox(var_A4, 0, var_B4, var_D4, var_F4)
  loc_403EE0:     GoTo loc_403F14
  loc_badkey:     ' Referenced from: 403D50
  loc_badkey:     ' Referenced from: 403C89
  loc_badkey:     ' Referenced from: 403BA4
  loc_badkey:     ' Referenced from: 4039B6
  loc_badkey:   End If
  loc_badkey: End If
  loc_403EE9: var_E4 = vbNullString
  loc_403EEE: var_B4 = var_E4
  loc_403EF9: var_C4 = "Sorry, This key is not valid."
  loc_403EFE: var_A4 = var_C4
  loc_403F04: MsgBox(var_A4, 0, var_B4, var_D4, var_F4)
  loc_403F14: ' Referenced from: 403EE0
  loc_403F14: Exit Sub
End Sub
