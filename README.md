# tessViewer

**(c) Araki Syun 2015**

tessViewerは**OpenGL**及び[**OpenSubdiv**](http://graphics.pixar.com/opensubdiv/docs/intro.html)の使用方法を検証するためのプログラムです。  
読み込んだモデルに**OpenSubdiv**を使い、テッセレーションをかけて表示します。


## 動作環境  
 - Windows10 64bit  
 - OpenGL 4.5以上

## インストール  
1. **setup.exe**を実行
1. インストールパスを指定

**tessViewer.exe**を実行可能

## 操作方法
|||
|---|---|
| 視点回転		|マウス中ボタン|
| 視点平行移動	|Shift+マウス中ボタン|
| ズーム		|マウススクロール|
| Fov変更		|Shift + マウススクロール|
| 終了			|Escape|

## コマンドラインオプション
||||
|---|---|---|
| -h| [ --help ]						|ヘルプを表示						|
| -v| [ --version ]						|バージョン情報						|
| -o| [ --object ] arg					|表示モデル							|
|   | --log_output arg (=quiet)			|ログの出力先						|
|   | --log_level arg (=0)				|ログの詳細度						|
||||
| -f| [ --full_screen ] arg				|全画面表示							|
|   | --resolution_x arg				|横解像度							|
|   | --resolution_y arg				|縦解像度							|
|   | --fov arg							|Fov								|
|   | --vsync arg						|VSync								|
||||
| -g| [ --osd_patch_type_gregory ]		|Patch Type に GREGORY_BASIS を使用	|
| -p| [ --osd_patch_level_default ] arg	|デフォルトパッチレベル					|
|   | --osd_patch_level_max arg			|最大パッチレベル						|
| -t| [ --osd_tess_level_default ] arg	|デフォルトテッセレーション係数			|
|   | --osd_tess_level_max arg			|最大テッセレーション係数				|
||||
|   | --user_interface arg				|ユーザーインターフェイス				|
|   | --font_size arg					|フォントサイズ						|
|   | --font_color arg					|フォント色							|

## setting.json
コマンドライン引数の代わりに`setting.json`を使用できます。  
設定項目がコマンドライン引数と重複した場合、コマンドライン引数の値が優先されます。

## 使用ライブラリ
|ライブラリ|バージョン|
|---|---|
| boost-program-options	| 1.71.0	|
| GLEW					| 2.1.0-6	|
| GLFW					| 3.3-3		|
| GLM					| 0.9.9.5-3	|
| OpenSubdiv			| 3.4.0		|
| nlohmann-json			| 3.7.3		|
| fmt					| 6.0.0		|
| freetype				| 2.10.1-1	|
| OpenCV				| 3.4.7-2	|
| libjpeg-turbo			| 2.0.3		|
| liblzma				| 5.2.4-2	|
| libpng				| 1.6.37-5	|
| libwebp				| 1.0.2-7	|
| bzip2					| 1.0.6-4	|
| ptex					| 2.3.2		|
| tiff					| 4.0.10-8	|
| zlib					| 1.2.11-5	|
