
= ChainlitのUIを@<ruby>{癖, へき}に染める

== はじめに
LLMが一般社会に急速に浸透し、我々の生活やワークスタイルは大きく変化しました。日々の献立から専門的な分野の調査分析まで、普段使う言葉で対話しながら解決できるLLMは、もはや日常になくてはならないものになっています。まるで、先人たちがSF作品で描いたような世界が現実になりつつあります。
特にゲームに絞ると、このような人工知能との関わりを描いた作品は数多く思い当たります。「ナビ」と呼ばれる人工知能を使ってインターネットウイルスを駆逐したり、2つのポータルと人間の知恵を駆使して暴走した人工知能と対峙したり。最近では、人工知能と共闘してリモートで「ホロウ」と呼ばれる危険地帯を探索する便利屋を扱ったソーシャルゲームも登場しました。このような作品に触れてきた方は、もしかしたらLLMに対し、単なるツールを超えた人格としての親近感を覚える方もいるのではないでしょうか。少なくとも私はそうです。
このような作品では、世界観に合わせて近未来的なUIが導入されることが多く、その鮮烈なカラーリングと演出に心を躍らせたものです。他方、大衆向け・ことビジネス向けサービスのUIに目を向けると、シンプルさ・わかりやすさが重視されます。ChatGPTやClaude Code、Chainlitの標準デザインもシンプルな構造ですよね。実際、シンプルな方がビジネスのさまざまな場面でノイズになりにくいですし、操作のわかりやすさという点でも理にかなっています。しかし頭では理解しながらも、LLMというSF作品の夢を具現化したような存在が現れた今、それを扱うUIも創作の典型に寄せてみたら、日々の業務がちょっと楽しくなるのでは？という思いを常に抱えていました。そんな時にdittoさんにこの記事執筆のお話をいただき、実際に手を動かして作ってみることにしました。
フロントエンドの開発の経験はなく、学生時代に静的サイトの装飾を多少扱った程度ですが、そのレベルでもできる範囲で実装してみます。

== この記事の取扱範囲と資料について
=== 取扱範囲・取り扱わない範囲
この記事ではChainlitに対するCSS, JSの適用による要素の変更を取り扱います。
想定レベルは「普段pythonやその他の言語を利用して分析・開発を行っているひと」
下記の項目は取り扱いません。
- CSS, JSの専門的な解説
- tailwind(ChainlitはShadcn/tailwindベースですが今回は利用しません)

=== 参考資料について
題材の性質上、この記事にはカラーで見た方が箇所がたびたび登場します。ただ、書籍は白黒印刷のため、下記のサイトに参考資料を掲載しました。
記事内でも必要な箇所にサイトに遷移するQRコードを掲載しますので、スマホ片手にご覧ください。
link
QR Code

== ChainlitのUI変更の仕様を知る
ChainlitはLLMとの会話アプリを簡単に実装することを主眼に作られています。フロントエンド開発の知識が乏しくてもアプリを構築できるよう、標準で使いやすいチャットUIが用意されています。これらはReact + Tailwind + shadcn/uiの構成でChainlit内部に実装されており、通常のプロジェクト作成時にはフロントエンドのソースコードはプロジェクトディレクトリに生成されません。
ただし、簡単なデザイン変更はサポートされています。具体的には、config.tomlに装飾用のファイル（css, js, faviconなど）のpathを書き込むことで、標準UIのスタイルを上書きできます。
整理すると、HTMLは編集不可、CSSとJSは適用可能という構図です。そのため、@<b>{標準UIをCSSで装飾しつつ、必要であればjsでHTMLの要素を編集する}という作戦で実装することにします。

== 目標
実装にあたり、Figmaでトンマナ案とデザイン案を作成しました。カラー版はリンクをご覧ください。
あくまでも案として、実装による多少の差異は許容することにします。

image, QR

また、今回の実装にあたり、円をモチーフにしたロゴを作成しました。最終的にはこのロゴを回転させます。

image

== 変更していこう

=== ロゴを変更する
手始めに、ロゴを変えていきましょう。このロゴはfaviconにも使われます。まず、.chainlitディレクトリと同じ階層にpublicディレクトリを作り、faviconとロゴに使うファイルを配置します。

//emlist[config.yamlとpublic]{
その他のディレクトリは省略
.chainlit
├── translations/ …省略
└── config.yaml

public/
└── logo.svg
//}

その後、config.yamlに移動し、logo_file_urlのコメントアウトを解除します。そして配置したロゴファイルのpathを入力すれば完了です。簡単！ついでに、AIの返答メッセージの横に表示されるアイコンも同じファイルにしておきましょう。

//emlist[config.yamlの編集][python]{
# Load assistant logo directly from URL.
logo_file_url = "/public/logo.svg"

# Load assistant avatar image directly from URL.
default_avatar_file_url = "/public/logo.svg"
//}

補足として、config.yamlには他にも設定項目があります。この記事では特に扱いませんが、これらを使えば標準UIのデザインをさらにカスタマイズできるでしょう。

//emlist[その他の設定項目例][python]{
# Custom login page image, relative to public directory or external URL
login_page_image = "/public/custom-background.jpg"

# Custom login page image filter (Tailwind internal filters, no dark/light variants)
login_page_image_filter = "brightness-50 grayscale"
login_page_image_dark_filter = "contrast-200 blur-sm"

# Specify a custom meta image url.
custom_meta_image_url = "https://chainlit-cloud.s3.eu-west-3.amazonaws.com/logo/chainlit_banner.png"
//}

=== 見出しやボタンの表記を変える
.chainlitの配下にtranslationsというディレクトリと多数のjsonが自動生成されます。これらは、ChainlitのUIで使用されるラベルやボタン、メニューなどのテキストの翻訳辞書です。これらの翻訳ファイルを編集することで、UIの表示文言を任意の言葉に変更することができます。デフォルトでは「ログイン」「パスワード」など、日本語を使った表記にしてくれているのですが、今回は見た目重視で英単語で表記されるように変更します。

//emlist[ja.jsonの設定][json]{
    "auth": {
        "login": {
            "title": "SYSTEM LOGIN",
            "form": {
                "email": {
                    "label": "USER ID",
                    "required": "USER ID IS REQUIRED / USER ID が必要です",
                    "placeholder": "USER ID"
                },
                "password": {
                    "label": "PASSWORD",
                    "required": "PASSWORD IS REQUIRED / PASSWORD が必要です",
                    "placeholder": "Password"
                },
                "actions": {
                    "signin": "LOGIN"
                },
                "alternativeText": {
                    "or": "\u307e\u305f\u306f"
                }
            },
//}


ここまでで、ロゴと表記を変更できました。今の見た目はこのようになっています。

//image[seahawk-default-login][変更前][scale=0.35]
//image[seahawk-customized-login][変更後][scale=0.35]


=== 静的要素を変更する

==== 初期設定とcustom_themeについて
ここからは、CSSを編集して静的要素を変更していきます。そのために、public内部にcssファイル, theme.jsonというファイルを作成し、config.yamlを編集します。ついでに、後ほど使うjsファイルも作成し、設定もしておきましょう。
theme.json は、ChainlitのUIで使用されるカラーパレットやデザイントークンを定義するためのファイルです。
このファイルを利用することで、アプリケーション全体の配色やUIテーマを統一することができます。
custom_css や custom_js は .chainlit/config.tomlにコメント付きの設定例が記載されていますが、custom_themeの設定はテンプレートに含まれていない場合があります。そのため、テーマを利用する場合は config.tomlにcustom_themeの設定を手動で追加し、theme.jsonを自分で作成する必要があります。

//emlist[cssファイル作成]{
その他のディレクトリは省略
.chainlit
├── translations/ …省略
└── config.yaml

public/
├── logo.svg
├── theme.json <- 新規作成
├── effects.js <- 新規作成
└── stylesheet.css <- 新規作成
//}

//emlist[config.yamlの編集][python]{
custom_theme = "/public/theme.json"

# Specify a CSS file that can be used to customize the user interface.
# The CSS file can be served from the public directory or via an external link.
custom_css = "/public/stylesheet.css"

# Specify a JavaScript file that can be used to customize the user interface.
# The JavaScript file can be served from the public directory.
custom_js = "/public/effects.js"
//}

ここに、あらかじめ決めておいたトンマナを記載しておきます。詳細はサンプルコードをご覧ください。
//emlist[theme.jsonのサンプル][json]{
{
  "custom_fonts": [],
  "variables": {
    "light": {
      "--background": "0 0% 100%",
      "--foreground": "0 0% 5%",
      ...
    },
    "dark": {
      "--background": "222 47% 6%",
      "--foreground": "222 40% 96%",
      ...
    }
  }
}
//}

==== ログイン画面の変更
基本的に、これからは@<b>{selectorを特定し、cssやjsで書き換える}という作業を続けていきます。
目標のデザインを実現する上で、大きな改造が必要なのは下記の2つです。この事例を中心に、対象要素の紹介とその操作を紹介します。

* 標準UIの背景のロゴを消し、入力欄を中央に配置する
* ロゴを入力欄の上部に表示する

===== 標準UIのロゴを消して、入力欄を中央に配置する
まず、ログイン画面の判定のために、下記のセレクタを利用します。
//emlist[]{
body:has(input[type="password"]):has(button[type="submit"])
//}

その上で、標準UIの背景画像は下記のセレクタで指定できるので、まるっと消します。
//emlist[]{
body:has(input[type="password"]):has(button[type="submit"])
  img.absolute.inset-0.h-full.w-full.object-cover {
  display: none !important;
}
//}

続いて、フォームを中央に配置する処理です。その上でフォームを含むrootを全面に広げ、中央配置します。その後、フォームを持つ要素をセンター寄せします。
//emlist[]{
body:has(input[type="password"]):has(button[type="submit"]) #root,
body:has(input[type="password"]):has(button[type="submit"]) main{
  ...
  width: 100%;
  place-items: center;
  ...
}

body:has(input[type="password"]):has(button[type="submit"])
  main:has(form) {
  ...
  justify-items: center !important;
  align-items: center !important;
  ...
}

body:has(input[type="password"]):has(button[type="submit"])
  main:has(form) > :has(form) {
  ...
  place-items: center !important;
  ...
}
//}

===== ロゴを入力欄の上部に表示する
ロゴを画面の中心かつ入力欄の上に配置するため、logo.svgを使った要素を探して、センター寄せします。

//emlist[]{
body:has(input[type="password"]):has(button[type="submit"])
  :is(img.logo, img[src="/public/logo.svg"], img[src="/logo"]) {
  display: block !important;
  width: 180px;
  height: auto;
  margin: 0 auto 24px auto !important;
}
//}

===== ボタン
あとはフォームの要素や見出し、ボタンを装飾します。下記は入力ボックスの例です。ログイン画面全体のinputを指定しており、同じ要領でbuttonも操作をしています。

//emlist[]{
body:has(input[type="password"]):has(button[type="submit"]) input {
  width: 100%;
  height: 42px;
  background: hsl(var(--background));
  color: hsl(var(--foreground));
  border: 1px solid hsl(var(--border));
  transition: border-color 0.2s ease, box-shadow 0.2s ease, filter 0.2s ease;
  border-radius: 0 !important;
  padding: 0 14px;
  outline: none;
  box-shadow:
    0 0 10px hsl(var(--primary) / 0.12);
}
//}

これらの処理により、目標の見た目に近づきました。
//image[seahawk-customized-login-2][良い感じになってきた][scale=0.5]

==== トーク画面の変更
ログイン画面に比べて、トーク画面の変更は比較的シンプルです。トーク画面の判定には下記のセレクタを利用します。

//emlist[]{
body:not(:has(input[type="password"]):has(button[type="submit"]))
//}

トーク画面には入力ボックス・ユーザーバブル・アシスタントバブル（LLM側の返答）があり、それぞれのカスタマイザは下記の通りです。アシスタントバブルはユーザーバブルに該当するclassの除外により分別しています。

//emlist[]{
/* 入力ボックスのselector */
body:not(:has(input[type="password"]):has(button[type="submit"])) #message-composer

/* ユーザーバブルのselector */
body:not(:has(input[type="password"]):has(button[type="submit"]))
  div.px-5.py-2\.5.relative.bg-accent.rounded-3xl.max-w-\[70\%\].flex-grow-0

/* アシスタントバブルのselector */
body:not(:has(input[type="password"]):has(button[type="submit"]))
  .message-content:not(.px-5.py-2\.5.relative.bg-accent.rounded-3xl.max-w-\[70\%\].flex-grow-0 .message-content)
//}

===== ユーザーバブルの装飾
標準UIでは、ユーザーバブルには角丸四角の装飾が施されています。
//image[seahawk-default-bubble][標準のバブルデザイン][scale=0.5]

実装ではこの要素を非表示にした上で、枠の装飾を適用しています。
//emlist[]{
body:not(:has(input[type="password"]):has(button[type="submit"]))
  div.px-5.py-2\.5.relative.bg-accent.rounded-3xl.max-w-\[70\%\].flex-grow-0 .message-content {
  background: transparent !important;
  border: 0 !important;
  ...
}
//}

===== アシスタントバブルの装飾
ユーザーバブルとは異なり、左にアイコン、その横に返答メッセージが表示されます。囲み枠はありません。
枠自体の適用はユーザーバブルと同様ですが、アイコンを含む分の位置調整が必要になります。

//emlist[]{
span.relative.flex.shrink-0.overflow-hidden.rounded-full[data-state]:has(> img[alt="Avatar for Assistant"]) img[alt="Avatar for Assistant"] {
  ...
  left: -30px;
  ...
}
//}

以上の編集を終えて、静的デザインは完成です。
//image[seahawk-customized-talk-1][トーク初期画面][scale=0.5]
//image[seahawk-customized-talk-2][トーク中の画面][scale=0.5]

=== 動的要素を変更する

==== ダイアログに動きをつける

==== トーク画面の変更

=== 完成！

== 終わりに



#@# 画像はarticlesフォルダの下のimagesフォルダに入れて、こんな風に指定すると表示されます。（@<img>{sample-diagram}）

#@# //image[sample-diagram][画像はここに表示される][scale=0.8]{
#@# //}

#@# 注釈を書くこともできます。@<fn>{footnote-sample}

#@# //footnote[footnote-sample][注釈はこんな風に表示される]

#@# Re:VIEWの文法について詳しくは、Re:VIEW フォーマットガイドを参照してください。

#@#  * Re:VIEW フォーマットガイド
#@#  ** @<href>{https://github.com/kmuto/review/blob/master/doc/format.ja.md}

