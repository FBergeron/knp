echo off

if not exist bunruidb.txt goto error

perl bgh.pl < bunruidb.txt > bgh.dat
del /F bgh.db
make_db bgh.db < bgh.dat
del /F sm2code.db
make_db sm2code.db < sm2code.dat

echo ���ތ�b�\�f�[�^�x�[�X���쐬����܂����B
goto end

:error
echo ���ތ�b�\�f�[�^�x�[�X�̍쐬�Ɏ��s���܂����B
goto end

:end
