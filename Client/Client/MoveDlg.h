#pragma once
class MoveDlg
{
public:
	MoveDlg();
	~MoveDlg();
	static void OnSize(const CDialog *dlg, CRect &rect, UINT nType, int cx, int cy);
	static void ChangeSize(const CDialog *dlg, CRect &rect, UINT nID, int x, int y);
};

