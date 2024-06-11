/*
LRESULT CALLBACK Normalization(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
      case WM_INITDIALOG:
         SendDlgItemMessage(hDialog, IDC_NORM_SLIDER, TBM_SETRANGE, 0, MAKELPARAM(0, 100));
         SendDlgItemMessage(hDialog, IDC_NORM_SLIDER, TBM_SETTICFREQ, 50, 0);
         SendDlgItemMessage(hDialog, IDC_NORM_SLIDER, TBM_SETPOS, 1, Norm_Ratio);
         sprintf(szTemp, "%d", Norm_Ratio);
         SetDlgItemText(hDialog, IDC_NORM, szTemp);

         ShowWindow(hDialog, SW_SHOW);

         if (Normalization_Flag)
            SendDlgItemMessage(hDialog, IDC_NORM_CHECK, BM_SETCHECK, BST_CHECKED, 0);
         return true;

      case WM_COMMAND:
         switch (LOWORD(wParam))
         {
            case IDC_NORM_CHECK:
               if (SendDlgItemMessage(hDialog, IDC_NORM_CHECK, BM_GETCHECK, 1, 0)==BST_CHECKED)
               {
                  CheckMenuItem(hMenu, IDM_NORM, MF_CHECKED);
                  Normalization_Flag = true;
               }
               else
               {
                  CheckMenuItem(hMenu, IDM_NORM, MF_UNCHECKED);
                  Normalization_Flag = false;
               }
               break;

            case IDCANCEL:
               DestroyWindow(hDialog);
               hNormDlg = NULL;
               return true;
         }
         break;

      case WM_HSCROLL:
         if (GetWindowLong((HWND)lParam, GWL_ID)==IDC_NORM_SLIDER)
         {
            Norm_Ratio = SendDlgItemMessage(hDialog, IDC_NORM_SLIDER, TBM_GETPOS, 0, 0);
            sprintf(szTemp, "%d", Norm_Ratio);
            SetDlgItemText(hDialog, IDC_NORM, szTemp);
         }
         break;
   }
   return false;
}
*/
